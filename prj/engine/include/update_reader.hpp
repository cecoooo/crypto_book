#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <utility>
#include <filesystem>
#include "book.hpp"

class UpdatesReader
{
private:
    std::vector<char> buffer;
    const char* cursor=nullptr;
    const char* fileEnd=nullptr;
    const char* lineEnd=nullptr;
    void applyBidOrAskUpdates(Book* book, const char updateType);
public:
    UpdatesReader(const std::filesystem::path& filePath);
    bool endOfFile();
    void getLine();
    std::string_view getSymbolFromLine();
    void applyUpdates(Book* book);
    void moveToNewLine();
};