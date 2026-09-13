#include "test_suites.hpp"
#include "book.hpp"
#include "book_container.hpp"
#include "price_level_container.hpp"
#include <vector>

void runBoundaryTests(TestRunner& runner)
{
    runner.run("empty price level container", []
    {
        PriceLevelContainer levels(ORDER_DESC, 0);

        levels.compact();

        requireEqual(
            static_cast<std::size_t>(0),
            levels.levelsCount(),
            "Empty container has active levels.");
        requireEqual(
            static_cast<std::size_t>(0),
            levels.pendingUpdatesCount(),
            "Empty container has pending updates.");
        require(levels.getFirst() == nullptr, "Empty container has a best level.");
    });

    runner.run("empty book has no best prices", []
    {
        Book book;

        require(book.bestBid() == nullptr, "Empty book has a best bid.");
        require(book.bestAsk() == nullptr, "Empty book has a best ask.");
    });

    runner.run("delete missing price does not create a level", []
    {
        PriceLevelContainer levels(ORDER_ASC, 0);

        levels.addOrUpdateItem({100, 0});
        levels.compact();

        requireEqual(
            static_cast<std::size_t>(0),
            levels.levelsCount(),
            "Deleting a missing price created a level.");
        require(levels.getFirst() == nullptr, "Missing deletion created a best level.");
    });

    runner.run("zero buffer size uses minimum threshold", []
    {
        PriceLevelContainer levels(ORDER_DESC, 0, 0);

        levels.addOrUpdateItem({100, 5});

        requireEqual(
            static_cast<std::size_t>(0),
            levels.pendingUpdatesCount(),
            "Zero buffer size did not compact immediately.");
        requireEqual(
            static_cast<std::size_t>(1),
            levels.levelsCount(),
            "Inserted level is missing.");
        requireEqual(100ULL, levels.getFirst()->price, "Incorrect best price.");
    });

    runner.run("repeated compactions keep the latest value", []
    {
        PriceLevelContainer levels(ORDER_DESC, 0, 2);

        levels.addOrUpdateItem({100, 1});
        levels.addOrUpdateItem({100, 2});
        levels.addOrUpdateItem({100, 0});
        levels.addOrUpdateItem({100, 7});

        requireEqual(
            static_cast<std::size_t>(1),
            levels.levelsCount(),
            "Repeated compactions created duplicate levels.");
        require(levels.getFirst() != nullptr, "Updated level is missing.");
        requireEqual(
            7ULL,
            levels.getFirst()->quantity,
            "Latest update was not preserved.");
    });

    runner.run("remove missing book does nothing", []
    {
        BookContainer books(2);
        books.addBook("BTCUSDT");

        books.removeBook("MISSING");

        requireEqual(
            static_cast<std::size_t>(1),
            books.booksCount(),
            "Removing a missing book changed the container.");
        const BookContainer& constBooks = books;
        require(
            constBooks.getBook("BTCUSDT") != nullptr,
            "Existing book was removed.");
    });
}
