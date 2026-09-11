#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <utility>
#include "book.hpp"

class SnapshotReader
{
private:
    std::vector<char> buffer;
    const char* cursor=nullptr;
    const char* fileEnd=nullptr;
    void loadBidOrAskFromSnapshot(Book& book, std::string_view bidsAsks);
public:
    SnapshotReader(const std::string& filePath);
    std::string getSymbolForNewSnapshot();
    void loadData(Book& book);
};