#pragma once
#include <iostream>
#include <string>
#include "book_container.hpp"
#include "update_reader.hpp"

class Engine
{
private:
    BookContainer& bookManager;
    std::unordered_map<std::string, Book*> bookCache;
    Book* getBookBySymbolFromCache(UpdatesReader& reader);
    void compactData();
public:
    Engine(BookContainer& bm);
    void loadUpdates(const std::string& updatesFilePath);
    void loadSnapshot(const std::string& snapshotFilePath);
};