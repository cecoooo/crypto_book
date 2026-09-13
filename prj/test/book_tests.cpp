#include "test_suites.hpp"
#include "book.hpp"
#include <vector>

void runBookTests(TestRunner& runner)
{
    runner.run("book maintains best bid and ask", []
    {
        Book book(3);
        std::vector<PriceLevel> bids{
            {100, 5},
            {102, 7},
            {101, 4}
        };
        std::vector<PriceLevel> asks{
            {105, 2},
            {103, 8},
            {104, 3}
        };

        book.loadInitialBids(std::move(bids));
        book.loadInitialAsks(std::move(asks));

        require(book.bestBid() != nullptr, "Best bid is missing.");
        require(book.bestAsk() != nullptr, "Best ask is missing.");
        requireEqual(102ULL, book.bestBid()->price, "Incorrect best bid.");
        requireEqual(103ULL, book.bestAsk()->price, "Incorrect best ask.");
    });

    runner.run("book applies updates to both sides", []
    {
        Book book(2);
        std::vector<PriceLevel> bids{{100, 5}, {99, 6}};
        std::vector<PriceLevel> asks{{101, 7}, {102, 8}};
        book.loadInitialBids(std::move(bids));
        book.loadInitialAsks(std::move(asks));

        book.updateBid(100, 0);
        book.updateBid(98, 9);
        book.updateAsk(101, 0);
        book.updateAsk(103, 10);
        book.Bids().compact();
        book.Asks().compact();

        requireEqual(99ULL, book.bestBid()->price, "Bid updates are incorrect.");
        requireEqual(102ULL, book.bestAsk()->price, "Ask updates are incorrect.");
        requireEqual(
            static_cast<std::size_t>(2),
            book.Bids().levelsCount(),
            "Incorrect bid count.");
        requireEqual(
            static_cast<std::size_t>(2),
            book.Asks().levelsCount(),
            "Incorrect ask count.");
    });
}
