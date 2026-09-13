#include "update_reader.hpp"
#include <fstream>
#include "pair_utils.hpp"

UpdatesReader::UpdatesReader(const std::filesystem::path& filePath)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);

    if (!file) {
        throw std::runtime_error("Could not open file: " + filePath.string());
    }

    const std::streamsize fileSize = file.tellg();

    if (fileSize < 0) {
        throw std::runtime_error("Could not determine file size: " + filePath.string());
    }

    buffer.resize(static_cast<std::size_t>(fileSize));

    if (!buffer.empty())
    {

        file.seekg(0, std::ios::beg);

        if (!file.read(buffer.data(), fileSize)) {
            throw std::runtime_error(
                "Failed to read complete file: " + filePath.string());
        }

        cursor = buffer.data();
        fileEnd = cursor + buffer.size();
    }
}

bool UpdatesReader::endOfFile()
{
    return cursor >= fileEnd;
}

void UpdatesReader::getLine()
{
    while (cursor < fileEnd && *cursor != '{') cursor++;

    lineEnd = cursor;
    while (lineEnd < fileEnd && *lineEnd != '\n') lineEnd++;
}

void UpdatesReader::moveToNewLine()
{
    cursor = lineEnd+1;
}

std::string_view UpdatesReader::getSymbolFromLine()
{
    const char* scan = cursor;

    while (scan < lineEnd && !(*scan == 's' && *(scan+1) == '"')) {
        scan++;
    }
    
    if (scan < lineEnd) {
        while (scan < lineEnd && *scan != ':') scan++;
        while (scan < lineEnd && *scan != '"') scan++; 
        if (scan < lineEnd) {
            const char* symStart = ++scan;
            while (scan < lineEnd && *scan != '"') scan++; 
            if (scan < lineEnd) {
                return std::string_view(symStart, scan - symStart);
            }
        }
    }
    return "";
}

void UpdatesReader::applyUpdates(Book* book)
{
    applyBidOrAskUpdates(book, 'b');
    applyBidOrAskUpdates(book, 'a');
}

void UpdatesReader::applyBidOrAskUpdates(Book* book, const char updateType)
{
    const char* scan = cursor;
    while (scan < lineEnd && !(*scan == updateType && *(scan+1) == '"')) scan++;
    while (scan < lineEnd) {
        while (scan < lineEnd && *scan != '[' && *scan != ']') scan++;
        if (scan >= lineEnd || (*scan == ']' && *(scan+1) == ']')) break;
        std::string_view pStr, qStr;
        if (extractNextPair(scan, lineEnd, pStr, qStr)) {
            std::uint64_t price = parseFixedPoint(pStr);
            std::uint64_t quantity = parseFixedPoint(qStr);
            switch (updateType)
            {
            case 'b':
                book->updateBid(price, quantity);
                break;
            case 'a':
                book->updateAsk(price, quantity);
                break;
            }
        }
    }
}
