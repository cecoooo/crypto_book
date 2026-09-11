#include "engine.hpp"
#include <algorithm>
#include <chrono>
#include "update_reader.hpp"
#include "snapshot_reader.hpp"

Engine::Engine(BookContainer& bm): bookManager{bm}{}

void Engine::loadUpdates(const std::string& updatesFilePath)
{
    UpdatesReader reader(updatesFilePath);

    auto start = std::chrono::high_resolution_clock::now();

    while (!reader.endOfFile())
    {
        reader.getLine();
        
        Book* book = getBookBySymbolFromCache(reader);
        if(!book) break;
        reader.applyUpdates(book);
        reader.moveToNewLine();
        
    }
    compactData();

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Processing updates time (in seconds): " << duration.count() / 1000000.0 << std::endl;
}

void Engine::loadSnapshot(const std::string& snapshotFilePath)
{
    auto startParse = std::chrono::high_resolution_clock::now();

    SnapshotReader reader(snapshotFilePath);

    auto stopParse = std::chrono::high_resolution_clock::now();
    auto durationParse = std::chrono::duration_cast<std::chrono::microseconds>(stopParse - startParse);
    std::cout << "Parse data to vector time (in seconds): " << durationParse.count() / 1000000.0 << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    std::string symbol = reader.getSymbolForNewSnapshot();
    std::cout << "Symbol: " << symbol << std::endl;

    bookManager.addBook(symbol);
    auto& book = bookManager.getBook(symbol);

    reader.loadData(book);

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Filling DSs time (in seconds): " << duration.count() / 1000000.0 << std::endl;
    
}

Book* Engine::getBookBySymbolFromCache(UpdatesReader& reader)
{ 
    std::string_view symbolView = reader.getSymbolFromLine();
    Book* bookPtr = nullptr;
    if (!symbolView.empty()) {
        auto it = bookCache.find(std::string(symbolView));
        if (it != bookCache.end()) {
            bookPtr = it->second; 
        } else {
            std::string symbolStr(symbolView);
            bookPtr = &bookManager.getBook(symbolStr);
            bookCache[symbolStr] = bookPtr;
        }
    }
    return bookPtr;
}

void Engine::compactData()
{
    for (const auto& [key, value] : bookCache) {
        value->Asks().compact();
        value->Bids().compact();
    }
    bookCache.clear();
}