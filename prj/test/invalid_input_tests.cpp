#include "test_suites.hpp"
#include "book_container.hpp"
#include "engine.hpp"
#include "price_level_container.hpp"
#include "snapshot_reader.hpp"
#include "update_reader.hpp"
#include <stdexcept>
#include <string>

void runInvalidInputTests(TestRunner& runner)
{
    runner.run("invalid order value throws", []
    {
        bool exceptionThrown = false;

        try
        {
            PriceLevelContainer levels("INVALID", 0);
        }
        catch (const std::invalid_argument&)
        {
            exceptionThrown = true;
        }

        require(exceptionThrown, "Invalid order value did not throw.");
    });

    runner.run("missing snapshot file throws", []
    {
        bool exceptionThrown = false;

        try
        {
            SnapshotReader reader(
                std::string(TEST_DATA_DIR) + "/missing_snapshot.json");
        }
        catch (const std::runtime_error&)
        {
            exceptionThrown = true;
        }

        require(exceptionThrown, "Missing snapshot file did not throw.");
    });

    runner.run("missing updates file throws", []
    {
        bool exceptionThrown = false;

        try
        {
            UpdatesReader reader(
                std::string(TEST_DATA_DIR) + "/missing_updates.txt");
        }
        catch (const std::runtime_error&)
        {
            exceptionThrown = true;
        }

        require(exceptionThrown, "Missing updates file did not throw.");
    });

    runner.run("snapshot without symbol throws", []
    {
        const std::string file =
            std::string(TEST_DATA_DIR) + "/snapshot_without_symbol.json";
        SnapshotReader reader(file);
        bool exceptionThrown = false;

        try
        {
            reader.getSymbolForNewSnapshot();
        }
        catch (const std::runtime_error&)
        {
            exceptionThrown = true;
        }

        require(exceptionThrown, "Snapshot without symbol did not throw.");
    });

    runner.run("update for unknown symbol throws", []
    {
        const std::string snapshotFile =
            std::string(TEST_DATA_DIR) + "/snapshot.json";
        const std::string updatesFile =
            std::string(TEST_DATA_DIR) + "/updates_unknown_symbol.txt";
        BookContainer books(2);
        Engine engine(books);
        engine.loadSnapshot(snapshotFile);
        bool exceptionThrown = false;

        try
        {
            engine.loadUpdates(updatesFile);
        }
        catch (const std::out_of_range&)
        {
            exceptionThrown = true;
        }

        require(exceptionThrown, "Unknown update symbol did not throw.");
    });
}
