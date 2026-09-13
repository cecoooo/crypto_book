#include "benchmark_memory.hpp"
#include "book.hpp"
#include "book_container.hpp"
#include "engine.hpp"
#include "pair_utils.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
    using Clock = std::chrono::steady_clock;

    constexpr std::uint64_t BID_BASE_PRICE = 1000000000000ULL;
    constexpr std::uint64_t ASK_BASE_PRICE = 2000000000000ULL;
    constexpr std::uint64_t INITIAL_QUANTITY = 1000000000ULL;

    struct Configuration
    {
        std::string mode="data";
        std::string scenario="all";
        std::string snapshotFile;
        std::string updatesFile;
        std::string logFile="benchmark_logs/performance.txt";
        std::size_t levels=250000;
        std::size_t updates=1000000;
        std::size_t runs=5;
        std::size_t bufferSize=BUFFER_MAX_SIZE;
    };

    struct Update
    {
        std::uint64_t price;
        std::uint64_t quantity;
        bool bid;
    };

    struct Result
    {
        std::string mode;
        std::string scenario;
        std::size_t run=0;
        std::size_t levelsPerSide=0;
        std::size_t workUnits=0;
        std::string unit;
        std::size_t bufferSize=0;
        double elapsedMilliseconds=0.0;
        double throughputPerSecond=0.0;
        double averageNanoseconds=0.0;
        std::size_t rssBeforeBytes=0;
        std::size_t rssAfterBytes=0;
        std::size_t peakRssBytes=0;
        std::size_t privateBytes=0;
        std::size_t allocatedBytes=0;
        std::size_t activeLevels=0;
        std::size_t pendingUpdates=0;
        std::uint64_t checksum=0;
    };

    class NullBuffer : public std::streambuf
    {
    protected:
        int overflow(int character) override
        {
            return traits_type::not_eof(character);
        }
    };

    class CoutSilencer
    {
    private:
        NullBuffer buffer;
        std::streambuf* previousBuffer;
    public:
        CoutSilencer(): previousBuffer{std::cout.rdbuf(&buffer)}{}

        ~CoutSilencer()
        {
            std::cout.rdbuf(previousBuffer);
        }
    };

    std::size_t parseSize(const std::string& text, const std::string& option)
    {
        std::size_t parsed = 0;
        const unsigned long long value = std::stoull(text, &parsed);

        if (parsed != text.size() ||
            value > std::numeric_limits<std::size_t>::max())
        {
            throw std::invalid_argument("Invalid value for " + option + ": " + text);
        }

        return static_cast<std::size_t>(value);
    }

    void printUsage()
    {
        std::cout
            << "Usage:\n"
            << "  order_book_benchmark [options]\n\n"
            << "Options:\n"
            << "  --mode data|files\n"
            << "  --scenario all|snapshot|replace|delete|insert|mixed\n"
            << "  --levels N\n"
            << "  --updates N\n"
            << "  --runs N\n"
            << "  --buffer N\n"
            << "  --snapshot FILE\n"
            << "  --updates-file FILE\n"
            << "  --log FILE\n"
            << "  --help\n";
    }

    Configuration parseArguments(int argc, char* argv[])
    {
        Configuration config;

        for (int index = 1; index < argc; index++)
        {
            const std::string argument = argv[index];

            if (argument == "--help")
            {
                printUsage();
                std::exit(0);
            }

            if (index + 1 >= argc)
            {
                throw std::invalid_argument("Missing value after " + argument);
            }

            const std::string value = argv[++index];

            if (argument == "--mode")
            {
                config.mode = value;
            }
            else if (argument == "--scenario")
            {
                config.scenario = value;
            }
            else if (argument == "--levels")
            {
                config.levels = parseSize(value, argument);
            }
            else if (argument == "--updates")
            {
                config.updates = parseSize(value, argument);
            }
            else if (argument == "--runs")
            {
                config.runs = parseSize(value, argument);
            }
            else if (argument == "--buffer")
            {
                config.bufferSize = parseSize(value, argument);
            }
            else if (argument == "--snapshot")
            {
                config.snapshotFile = value;
            }
            else if (argument == "--updates-file")
            {
                config.updatesFile = value;
            }
            else if (argument == "--log")
            {
                config.logFile = value;
            }
            else
            {
                throw std::invalid_argument("Unknown option: " + argument);
            }
        }

        if (config.runs == 0 || config.bufferSize == 0)
        {
            throw std::invalid_argument("--runs and --buffer must be greater than zero");
        }

        if (config.mode != "data" && config.mode != "files")
        {
            throw std::invalid_argument("--mode must be data or files");
        }

        if (config.mode == "files" &&
            (config.snapshotFile.empty() || config.updatesFile.empty()))
        {
            throw std::invalid_argument(
                "File mode requires --snapshot and --updates-file");
        }

        const std::vector<std::string> scenarios{
            "all", "snapshot", "replace", "delete", "insert", "mixed"};

        if (std::find(scenarios.begin(), scenarios.end(), config.scenario) ==
            scenarios.end())
        {
            throw std::invalid_argument("Unknown scenario: " + config.scenario);
        }

        return config;
    }

    std::vector<PriceLevel> createLevels(
        std::size_t count,
        std::uint64_t basePrice)
    {
        std::vector<PriceLevel> levels;
        levels.reserve(count);

        for (std::size_t index = 0; index < count; index++)
        {
            levels.emplace_back(
                basePrice + static_cast<std::uint64_t>(index),
                INITIAL_QUANTITY + static_cast<std::uint64_t>(index % 1000));
        }

        return levels;
    }

    Book createBook(std::size_t levels, std::size_t bufferSize)
    {
        Book book(levels, bufferSize);
        book.loadInitialBids(createLevels(levels, BID_BASE_PRICE));
        book.loadInitialAsks(createLevels(levels, ASK_BASE_PRICE));
        return book;
    }

    std::uint64_t nextRandom(std::uint64_t& state)
    {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        return state;
    }

    std::vector<Update> createUpdates(
        const std::string& scenario,
        std::size_t count,
        std::size_t levels)
    {
        std::vector<Update> updates;
        updates.reserve(count);

        const std::uint64_t levelRange =
            static_cast<std::uint64_t>(std::max<std::size_t>(1, levels));
        std::uint64_t randomState = 0x9E3779B97F4A7C15ULL;

        for (std::size_t index = 0; index < count; index++)
        {
            const bool bid = (index & 1U) == 0;
            const std::uint64_t basePrice = bid ? BID_BASE_PRICE : ASK_BASE_PRICE;
            const std::uint64_t existingPrice =
                basePrice + nextRandom(randomState) % levelRange;
            const std::uint64_t newPrice =
                basePrice + levelRange + 1U +
                static_cast<std::uint64_t>(index);

            if (scenario == "replace")
            {
                updates.push_back({
                    existingPrice,
                    INITIAL_QUANTITY + static_cast<std::uint64_t>(index + 1U),
                    bid});
            }
            else if (scenario == "delete")
            {
                updates.push_back({existingPrice, 0, bid});
            }
            else if (scenario == "insert")
            {
                updates.push_back({newPrice, INITIAL_QUANTITY, bid});
            }
            else
            {
                switch (index % 10U)
                {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        updates.push_back({
                            existingPrice,
                            INITIAL_QUANTITY +
                                static_cast<std::uint64_t>(index + 1U),
                            bid});
                        break;
                    case 5:
                    case 6:
                        updates.push_back({existingPrice, 0, bid});
                        break;
                    case 7:
                    case 8:
                        updates.push_back({newPrice, INITIAL_QUANTITY, bid});
                        break;
                    default:
                        updates.push_back({
                            basePrice + levelRange * 4U +
                                static_cast<std::uint64_t>(index),
                            0,
                            bid});
                        break;
                }
            }
        }

        return updates;
    }

    std::uint64_t calculateChecksum(const Book& book)
    {
        std::uint64_t checksum =
            static_cast<std::uint64_t>(book.Bids().levelsCount()) ^
            (static_cast<std::uint64_t>(book.Asks().levelsCount()) << 1U);

        if (const PriceLevel* bestBid = book.bestBid())
        {
            checksum ^= bestBid->price;
            checksum ^= bestBid->quantity;
        }

        if (const PriceLevel* bestAsk = book.bestAsk())
        {
            checksum ^= bestAsk->price;
            checksum ^= bestAsk->quantity;
        }

        return checksum;
    }

    Result runSnapshot(
        std::size_t run,
        std::size_t levels,
        std::size_t bufferSize)
    {
        auto bids = createLevels(levels, BID_BASE_PRICE);
        auto asks = createLevels(levels, ASK_BASE_PRICE);
        const MemoryUsage before = getMemoryUsage();

        const auto start = Clock::now();
        Book book(levels, bufferSize);
        book.loadInitialBids(std::move(bids));
        book.loadInitialAsks(std::move(asks));
        const auto stop = Clock::now();

        const MemoryUsage after = getMemoryUsage();
        const double elapsedNanoseconds =
            std::chrono::duration<double, std::nano>(stop - start).count();
        const std::size_t workUnits = levels * 2U;

        return {
            "data",
            "snapshot",
            run,
            levels,
            workUnits,
            "levels",
            bufferSize,
            elapsedNanoseconds / 1000000.0,
            workUnits > 0 ? workUnits * 1000000000.0 / elapsedNanoseconds : 0.0,
            workUnits > 0 ? elapsedNanoseconds / workUnits : 0.0,
            before.residentBytes,
            after.residentBytes,
            after.peakResidentBytes,
            after.privateBytes,
            book.Bids().allocatedBytes() + book.Asks().allocatedBytes(),
            book.Bids().levelsCount() + book.Asks().levelsCount(),
            book.Bids().pendingUpdatesCount() +
                book.Asks().pendingUpdatesCount(),
            calculateChecksum(book)};
    }

    Result runUpdates(
        const std::string& scenario,
        std::size_t run,
        std::size_t levels,
        std::size_t updateCount,
        std::size_t bufferSize)
    {
        Book book = createBook(levels, bufferSize);
        const std::vector<Update> updates =
            createUpdates(scenario, updateCount, levels);
        const MemoryUsage before = getMemoryUsage();

        const auto start = Clock::now();
        for (const Update& update : updates)
        {
            if (update.bid)
            {
                book.updateBid(update.price, update.quantity);
            }
            else
            {
                book.updateAsk(update.price, update.quantity);
            }
        }
        book.Bids().compact();
        book.Asks().compact();
        const auto stop = Clock::now();

        const MemoryUsage after = getMemoryUsage();
        const double elapsedNanoseconds =
            std::chrono::duration<double, std::nano>(stop - start).count();

        return {
            "data",
            scenario,
            run,
            levels,
            updateCount,
            "updates",
            bufferSize,
            elapsedNanoseconds / 1000000.0,
            updateCount > 0 ?
                updateCount * 1000000000.0 / elapsedNanoseconds : 0.0,
            updateCount > 0 ? elapsedNanoseconds / updateCount : 0.0,
            before.residentBytes,
            after.residentBytes,
            after.peakResidentBytes,
            after.privateBytes,
            book.Bids().allocatedBytes() + book.Asks().allocatedBytes(),
            book.Bids().levelsCount() + book.Asks().levelsCount(),
            book.Bids().pendingUpdatesCount() +
                book.Asks().pendingUpdatesCount(),
            calculateChecksum(book)};
    }

    std::uint64_t calculateChecksum(const BookContainer& books)
    {
        return static_cast<std::uint64_t>(books.booksCount()) ^
            (static_cast<std::uint64_t>(books.levelsCount()) << 1U) ^
            (static_cast<std::uint64_t>(books.pendingUpdatesCount()) << 2U);
    }

    Result runFiles(
        std::size_t run,
        const Configuration& config)
    {
        const std::size_t inputBytes =
            static_cast<std::size_t>(std::filesystem::file_size(config.snapshotFile)) +
            static_cast<std::size_t>(std::filesystem::file_size(config.updatesFile));
        const MemoryUsage before = getMemoryUsage();

        const auto start = Clock::now();
        BookContainer books(2000);
        Engine engine(books);
        {
            CoutSilencer silencer;
            engine.loadSnapshot(config.snapshotFile);
            engine.loadUpdates(config.updatesFile);
        }
        const auto stop = Clock::now();

        const MemoryUsage after = getMemoryUsage();
        const double elapsedNanoseconds =
            std::chrono::duration<double, std::nano>(stop - start).count();

        return {
            "files",
            "snapshot_and_updates",
            run,
            0,
            inputBytes,
            "bytes",
            BUFFER_MAX_SIZE,
            elapsedNanoseconds / 1000000.0,
            inputBytes > 0 ?
                inputBytes * 1000000000.0 / elapsedNanoseconds : 0.0,
            inputBytes > 0 ? elapsedNanoseconds / inputBytes : 0.0,
            before.residentBytes,
            after.residentBytes,
            after.peakResidentBytes,
            after.privateBytes,
            books.allocatedBytes(),
            books.levelsCount(),
            books.pendingUpdatesCount(),
            calculateChecksum(books)};
    }

    std::string createTimestamp()
    {
        const auto now = std::chrono::system_clock::now();
        const std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::tm value{};

#if defined(_WIN32)
        localtime_s(&value, &time);
#else
        localtime_r(&time, &value);
#endif

        std::ostringstream stream;
        stream << std::put_time(&value, "%Y-%m-%d %H:%M:%S");
        return stream.str();
    }

    void writeResult(const std::string& logFile, const Result& result)
    {
        const std::filesystem::path path(logFile);
        if (path.has_parent_path())
        {
            std::filesystem::create_directories(path.parent_path());
        }

        const bool writeHeader =
            !std::filesystem::exists(path) ||
            std::filesystem::file_size(path) == 0;
        std::ofstream output(logFile, std::ios::app);

        if (!output)
        {
            throw std::runtime_error("Cannot open log file: " + logFile);
        }

        if (writeHeader)
        {
            output
                << "ORDER BOOK BENCHMARK REPORT\n"
                << "All memory values are shown in bytes and MiB.\n"
                << "Peak RSS is the highest process RSS observed by the operating system.\n"
                << "======================================================================\n\n";
        }

        const double bytesInMiB = 1024.0 * 1024.0;
        const double rssBeforeMiB = result.rssBeforeBytes / bytesInMiB;
        const double rssAfterMiB = result.rssAfterBytes / bytesInMiB;
        const double rssDeltaMiB =
            (static_cast<double>(result.rssAfterBytes) -
             static_cast<double>(result.rssBeforeBytes)) / bytesInMiB;
        const double peakRssMiB = result.peakRssBytes / bytesInMiB;
        const double privateMiB = result.privateBytes / bytesInMiB;
        const double allocatedMiB = result.allocatedBytes / bytesInMiB;

        output
            << "BENCHMARK RESULT\n"
            << "----------------------------------------------------------------------\n"
            << "Timestamp              : " << createTimestamp() << '\n'
            << "Mode                   : " << result.mode << '\n'
            << "Scenario               : " << result.scenario << '\n'
            << "Run                    : " << result.run << '\n'
            << "Levels per side        : " << result.levelsPerSide << '\n'
            << "Work units             : " << result.workUnits << ' '
            << result.unit << '\n'
            << "Buffer size            : " << result.bufferSize << "\n\n"
            << "PERFORMANCE\n"
            << "----------------------------------------------------------------------\n"
            << std::fixed << std::setprecision(3)
            << "Elapsed time           : " << result.elapsedMilliseconds << " ms\n"
            << "Throughput             : " << result.throughputPerSecond << ' '
            << result.unit << "/second\n"
            << "Average time           : " << result.averageNanoseconds
            << " ns/" << result.unit << '\n';

        if (result.unit == "bytes")
        {
            output
                << "Input throughput       : "
                << result.throughputPerSecond / bytesInMiB << " MiB/second\n";
        }

        output
            << "\nMEMORY USAGE\n"
            << "----------------------------------------------------------------------\n"
            << "RSS before             : " << result.rssBeforeBytes
            << " bytes (" << rssBeforeMiB << " MiB)\n"
            << "RSS after              : " << result.rssAfterBytes
            << " bytes (" << rssAfterMiB << " MiB)\n"
            << "RSS change             : " << std::showpos << rssDeltaMiB
            << std::noshowpos << " MiB\n"
            << "Peak RSS               : " << result.peakRssBytes
            << " bytes (" << peakRssMiB << " MiB)\n"
            << "Private memory         : " << result.privateBytes
            << " bytes (" << privateMiB << " MiB)\n"
            << "Container allocation   : " << result.allocatedBytes
            << " bytes (" << allocatedMiB << " MiB)\n\n"
            << "FINAL STATE\n"
            << "----------------------------------------------------------------------\n"
            << "Active levels          : " << result.activeLevels << '\n'
            << "Pending updates        : " << result.pendingUpdates << '\n'
            << "Checksum               : " << result.checksum << '\n'
            << "======================================================================\n\n";
    }

    void printResult(const Result& result)
    {
        const double rssDeltaMiB =
            (static_cast<double>(result.rssAfterBytes) -
             static_cast<double>(result.rssBeforeBytes)) /
            (1024.0 * 1024.0);

        std::cout
            << std::left
            << std::setw(22) << result.scenario
            << " run=" << std::setw(2) << result.run
            << " time=" << std::fixed << std::setprecision(3)
            << std::setw(11) << result.elapsedMilliseconds << " ms"
            << " throughput=" << std::setprecision(2)
            << std::setw(14) << result.throughputPerSecond
            << ' ' << result.unit << "/s"
            << " average=" << std::setw(10)
            << result.averageNanoseconds << " ns/" << result.unit
            << " rss_delta=" << std::showpos << std::setprecision(2)
            << rssDeltaMiB << std::noshowpos << " MiB"
            << " allocated=" << std::setprecision(2)
            << result.allocatedBytes / (1024.0 * 1024.0) << " MiB"
            << '\n';
    }

    void runDataBenchmarks(const Configuration& config)
    {
        std::vector<std::string> scenarios;

        if (config.scenario == "all")
        {
            scenarios = {"snapshot", "replace", "delete", "insert", "mixed"};
        }
        else
        {
            scenarios = {config.scenario};
        }

        for (const std::string& scenario : scenarios)
        {
            for (std::size_t run = 1; run <= config.runs; run++)
            {
                Result result;
                if (scenario == "snapshot")
                {
                    result = runSnapshot(run, config.levels, config.bufferSize);
                }
                else
                {
                    result = runUpdates(
                        scenario,
                        run,
                        config.levels,
                        config.updates,
                        config.bufferSize);
                }

                printResult(result);
                writeResult(config.logFile, result);
            }
        }
    }

    void runFileBenchmarks(const Configuration& config)
    {
        for (std::size_t run = 1; run <= config.runs; run++)
        {
            const Result result = runFiles(run, config);
            printResult(result);
            writeResult(config.logFile, result);
        }
    }
}

int main(int argc, char* argv[])
{
    try
    {
        const Configuration config = parseArguments(argc, argv);

        std::cout
            << "Order book benchmark\n"
            << "Mode: " << config.mode << '\n'
            << "Runs: " << config.runs << '\n'
            << "Log: " << config.logFile << "\n\n";

        if (config.mode == "data")
        {
            runDataBenchmarks(config);
        }
        else
        {
            runFileBenchmarks(config);
        }

        std::cout << "\nBenchmark completed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Benchmark error: " << error.what() << '\n';
        return 1;
    }
}
