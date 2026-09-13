#include "test_suites.hpp"
#include "book_container.hpp"
#include <stdexcept>
#include <string>

void runBookContainerTests(TestRunner& runner)
{
    runner.run("add and retrieve books", []
    {
        BookContainer books(4);
        books.addBook("BTCUSDT");
        books.addBook("ETHUSDT");
        books.addBook("BTCUSDT");
        const BookContainer& constBooks = books;

        requireEqual(
            static_cast<std::size_t>(2),
            books.booksCount(),
            "Duplicate symbol created another book.");
        require(
            constBooks.getBook("BTCUSDT") != nullptr,
            "BTCUSDT book was not found.");
        require(
            constBooks.getBook("ETHUSDT") != nullptr,
            "ETHUSDT book was not found.");
    });

    runner.run("remove a book and preserve moved book", []
    {
        BookContainer books(3);
        books.addBook("FIRST");
        books.addBook("SECOND");
        books.addBook("THIRD");

        books.removeBook("SECOND");
        const BookContainer& constBooks = books;

        requireEqual(
            static_cast<std::size_t>(2),
            books.booksCount(),
            "Book was not removed.");
        require(
            constBooks.getBook("SECOND") == nullptr,
            "Removed book is still available.");
        require(
            constBooks.getBook("THIRD") != nullptr,
            "Moved book cannot be retrieved.");
    });

    runner.run("missing mutable book throws", []
    {
        BookContainer books(1);
        bool exceptionThrown = false;

        try
        {
            books.getBook("MISSING");
        }
        catch (const std::out_of_range&)
        {
            exceptionThrown = true;
        }

        require(exceptionThrown, "Missing book did not throw.");
    });

    runner.run("container supports two thousand symbols", []
    {
        BookContainer books(2000);

        for (std::size_t index = 0; index < 2000; index++)
        {
            books.addBook("SYMBOL_" + std::to_string(index));
        }

        const BookContainer& constBooks = books;

        requireEqual(
            static_cast<std::size_t>(2000),
            books.booksCount(),
            "Incorrect symbol count.");
        require(
            constBooks.getBook("SYMBOL_0") != nullptr,
            "First symbol is missing.");
        require(
            constBooks.getBook("SYMBOL_1999") != nullptr,
            "Last symbol is missing.");
    });
}
