#include "book_container.hpp"
#include <stdexcept>

BookContainer::BookContainer(std::size_t expectedSymbols)
{
    if (expectedSymbols > 0) {
        symbolIds.reserve(expectedSymbols);
        books.reserve(expectedSymbols);
    }
}

void BookContainer::addBook(const std::string_view id)
{
    std::string key{id};
    if(auto it = symbolIds.find(key); it == symbolIds.end())
    {
        symbolIds.emplace(key, static_cast<SymbolId>(books.size()));
        books.push_back(Book());
    }
}

Book& BookContainer::getBook(const std::string_view id)
{
    auto it = symbolIds.find(std::string(id));
    if (it == symbolIds.end()) {
        throw std::out_of_range("Symbol ID not found in BookContainer!");
    }
    return books[it->second];
}

const Book* BookContainer::getBook(const std::string_view id) const noexcept
{
    auto it = symbolIds.find(std::string(id));
    if (it == symbolIds.end()) {
        return nullptr;
    }
    return &books[it->second];
}

void BookContainer::removeBook(const std::string_view id)
{
    std::string key{id};
    auto it = symbolIds.find(key);
    if (it == symbolIds.end()) {
        return;
    }

    SymbolId indexToRemove = it->second;
    SymbolId lastIndex = static_cast<SymbolId>(books.size() - 1);

    if (indexToRemove != lastIndex) {
        for (auto& [sym, id] : symbolIds) {
            if (id == lastIndex) {
                id = indexToRemove; 
                break;
            }
        }
        std::swap(books[indexToRemove], books[lastIndex]);
    }

    books.pop_back();
    symbolIds.erase(it);
}

Book& BookContainer::addOrGetBook(const std::string_view id, std::size_t expectedLevels) {
    std::string key{id};
    auto [it, inserted] = symbolIds.emplace(key, static_cast<SymbolId>(books.size()));
    if (inserted) {
        books.push_back(Book(expectedLevels));
    }
    return books[it->second];
}

std::size_t BookContainer::booksCount() const noexcept
{
    return books.size();
}

std::size_t BookContainer::levelsCount() const noexcept
{
    std::size_t count = 0;
    for (const auto& book : books)
    {
        count += book.Bids().levelsCount();
        count += book.Asks().levelsCount();
    }
    return count;
}

std::size_t BookContainer::pendingUpdatesCount() const noexcept
{
    std::size_t count = 0;
    for (const auto& book : books)
    {
        count += book.Bids().pendingUpdatesCount();
        count += book.Asks().pendingUpdatesCount();
    }
    return count;
}

std::size_t BookContainer::allocatedBytes() const noexcept
{
    std::size_t bytes = books.capacity() * sizeof(Book);
    for (const auto& book : books)
    {
        bytes += book.Bids().allocatedBytes();
        bytes += book.Asks().allocatedBytes();
    }
    return bytes;
}
