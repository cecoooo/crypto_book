#include "snapshot_reader.hpp"
#include <fstream>
#include "pair_utils.hpp"

SnapshotReader::SnapshotReader(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);

    if (!file) {
        throw std::runtime_error("Could not open file: " + filePath);
    }

    const std::streamsize fileSize = file.tellg();

    if (fileSize < 0) {
        throw std::runtime_error("Could not determine file size: " + filePath);
    }

    buffer.resize(static_cast<std::size_t>(fileSize));

    if (!buffer.empty())
    {

        file.seekg(0, std::ios::beg);

        if (!file.read(buffer.data(), fileSize)) {
            throw std::runtime_error(
                "Failed to read complete file: " + filePath);
        }

        cursor = buffer.data();
        fileEnd = cursor + buffer.size();
    }
}

std::string SnapshotReader::getSymbolForNewSnapshot()
{
    std::string symbol = "";
    const char* symScan = cursor;
    while (symScan < fileEnd && !(*symScan == 's' && *(symScan+1) == 'y' && *(symScan+2) == 'm')) {
        symScan++;
    }
    
    if (symScan < fileEnd) {
        while (symScan < fileEnd && *symScan != ':') symScan++;
        while (symScan < fileEnd && *symScan != '"') symScan++;
        if (symScan < fileEnd) {
            const char* symStart = ++symScan;
            while (symScan < fileEnd && *symScan != '"') symScan++;
            if (symScan < fileEnd) {
                symbol = std::string(symStart, symScan - symStart);
            }
        }
    }

    if (symbol.empty()) {
        throw std::runtime_error("Symbol not found in snapshot format!");
    }
    return symbol;
}

void SnapshotReader::loadBidOrAskFromSnapshot(Book& book, std::string_view bidsAsks)
{
    std::vector<PriceLevel> initialLevels;
    initialLevels.reserve(buffer.size()/60);
    int c=0;
    std::cout << "Buffer size: " << std::to_string(buffer.size()) << std::endl;

    const char* scan = cursor;
    while (scan < fileEnd && !(*scan == bidsAsks[0] && *(scan+1) == bidsAsks[1] && *(scan+2) == bidsAsks[2])) scan++;


    while (scan < fileEnd && *scan != '[') scan++;
    if (scan < fileEnd) scan++;

    int openBrackets = 1;
    while (scan < fileEnd && openBrackets > 0) {
        if (*scan == '[') {
            std::string_view pStr, qStr;
            if (extractNextPair(scan, fileEnd, pStr, qStr)) {
                std::uint64_t price = parseFixedPoint(pStr);
                std::uint64_t quantity = parseFixedPoint(qStr);
                initialLevels.emplace_back(price, quantity);
                c++;
            }
        } else if (*scan == ']') {
            openBrackets--;
        }
        scan++;
    }
    if(bidsAsks == "asks")
    {
        book.loadInitialAsks(std::move(initialLevels));
    }
    else
    {
        book.loadInitialBids(std::move(initialLevels));
    }
    std::cout << "Valuye of c: " << std::to_string(c) << std::endl;
}

void SnapshotReader::loadData(Book& book)
{
    loadBidOrAskFromSnapshot(book, "bids");
    loadBidOrAskFromSnapshot(book, "asks");
}