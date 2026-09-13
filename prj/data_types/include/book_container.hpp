#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>
#include "book.hpp"

using SymbolId = std::uint32_t;

class BookContainer
{
private:
    std::unordered_map<std::string, SymbolId> symbolIds;
    std::vector<Book> books;
public:
    BookContainer(std::size_t expectedSymbols);
    void addBook(const std::string_view id);
    Book& getBook(const std::string_view id);
    const Book* getBook(const std::string_view id) const noexcept;
    void removeBook(const std::string_view id);
    Book& addOrGetBook(const std::string_view id, std::size_t expectedLevels);
    std::size_t booksCount() const noexcept;
    std::size_t levelsCount() const noexcept;
    std::size_t pendingUpdatesCount() const noexcept;
    std::size_t allocatedBytes() const noexcept;
};
