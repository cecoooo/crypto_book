#include "engine.hpp"
#include <algorithm>
#include <chrono>
#include "update_reader.hpp"
#include "snapshot_reader.hpp"

Engine::Engine(BookContainer& bm): bookManager{bm}{}

void Engine::loadUpdates(const std::string& updatesFilePath)
{
    UpdatesReader reader(updatesFilePath);

    while (!reader.endOfFile())
    {
        reader.getLine();
        
        Book* book = getBookBySymbolFromCache(reader);
        if(!book) break;
        reader.applyUpdates(book);
        reader.moveToNewLine();
        
    }
    compactData();
}

void Engine::loadSnapshot(const std::string& snapshotFilePath)
{
    SnapshotReader reader(snapshotFilePath);

    std::string symbol = reader.getSymbolForNewSnapshot();

    bookManager.addBook(symbol);
    auto& book = bookManager.getBook(symbol);

    reader.loadData(book);
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