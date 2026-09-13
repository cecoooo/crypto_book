#include "test_suites.hpp"
#include "book_container.hpp"
#include "engine.hpp"
#include "pair_utils.hpp"
#include <string>

void runEngineTests(TestRunner& runner)
{
    runner.run("engine loads snapshot and applies updates", []
    {
        const std::string snapshotFile =
            std::string(TEST_DATA_DIR) + "/snapshot.json";
        const std::string updatesFile =
            std::string(TEST_DATA_DIR) + "/updates.txt";

        BookContainer books(2);
        Engine engine(books);

        engine.loadSnapshot(snapshotFile);
        engine.loadUpdates(updatesFile);

        const BookContainer& constBooks = books;
        const Book* book = constBooks.getBook("TEST");
        require(book != nullptr, "Snapshot symbol was not loaded.");
        require(book->bestBid() != nullptr, "Best bid is missing.");
        require(book->bestAsk() != nullptr, "Best ask is missing.");
        requireEqual(
            parseFixedPoint("99"),
            book->bestBid()->price,
            "Incorrect best bid after updates.");
        requireEqual(
            parseFixedPoint("102"),
            book->bestAsk()->price,
            "Incorrect best ask after updates.");
        requireEqual(
            static_cast<std::size_t>(2),
            book->Bids().levelsCount(),
            "Incorrect bid count after updates.");
        requireEqual(
            static_cast<std::size_t>(2),
            book->Asks().levelsCount(),
            "Incorrect ask count after updates.");
        requireEqual(
            static_cast<std::size_t>(0),
            book->Bids().pendingUpdatesCount() +
                book->Asks().pendingUpdatesCount(),
            "Engine did not compact pending updates.");
    });
}
