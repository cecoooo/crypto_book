#include "test_suites.hpp"
#include "price_level_container.hpp"
#include <vector>

void runPriceLevelContainerTests(TestRunner& runner)
{
    runner.run("load and order bid levels", []
    {
        PriceLevelContainer levels(ORDER_DESC, 5);
        std::vector<PriceLevel> snapshot{
            {100, 1},
            {102, 2},
            {101, 3},
            {101, 7},
            {99, 0}
        };

        levels.loadInitialLevels(std::move(snapshot));

        requireEqual(
            static_cast<std::size_t>(3),
            levels.levelsCount(),
            "Incorrect active level count.");
        require(levels.getFirst() != nullptr, "Best bid is missing.");
        requireEqual(102ULL, levels.getFirst()->price, "Incorrect best bid.");

        levels.addOrUpdateItem({102, 0});
        levels.compact();

        requireEqual(101ULL, levels.getFirst()->price, "Bid deletion failed.");
        requireEqual(
            7ULL,
            levels.getFirst()->quantity,
            "Last duplicate snapshot value was not preserved.");
    });

    runner.run("load and order ask levels", []
    {
        PriceLevelContainer levels(ORDER_ASC, 4);
        std::vector<PriceLevel> snapshot{
            {103, 1},
            {101, 2},
            {102, 3},
            {100, 0}
        };

        levels.loadInitialLevels(std::move(snapshot));

        require(levels.getFirst() != nullptr, "Best ask is missing.");
        requireEqual(101ULL, levels.getFirst()->price, "Incorrect best ask.");
        requireEqual(
            static_cast<std::size_t>(3),
            levels.levelsCount(),
            "Zero-quantity snapshot level was not removed.");
    });

    runner.run("last update for a price wins", []
    {
        PriceLevelContainer levels(ORDER_DESC, 1);
        std::vector<PriceLevel> snapshot{{100, 1}};
        levels.loadInitialLevels(std::move(snapshot));

        levels.addOrUpdateItem({100, 2});
        levels.addOrUpdateItem({100, 0});
        levels.addOrUpdateItem({100, 5});
        levels.compact();

        requireEqual(
            static_cast<std::size_t>(1),
            levels.levelsCount(),
            "Duplicate updates created duplicate levels.");
        requireEqual(5ULL, levels.getFirst()->quantity, "Last update did not win.");
    });

    runner.run("zero quantity removes a level", []
    {
        PriceLevelContainer levels(ORDER_ASC, 1);
        std::vector<PriceLevel> snapshot{{100, 1}};
        levels.loadInitialLevels(std::move(snapshot));

        levels.addOrUpdateItem({100, 0});
        levels.compact();

        requireEqual(
            static_cast<std::size_t>(0),
            levels.levelsCount(),
            "Deleted level is still active.");
        require(levels.getFirst() == nullptr, "Empty container has a best level.");
    });

    runner.run("buffer threshold invokes compaction", []
    {
        PriceLevelContainer levels(ORDER_DESC, 1, 2);
        std::vector<PriceLevel> snapshot{{100, 1}};
        levels.loadInitialLevels(std::move(snapshot));

        levels.addOrUpdateItem({101, 2});
        requireEqual(
            static_cast<std::size_t>(1),
            levels.pendingUpdatesCount(),
            "Update was not buffered.");

        levels.addOrUpdateItem({102, 3});

        requireEqual(
            static_cast<std::size_t>(0),
            levels.pendingUpdatesCount(),
            "Automatic compaction was not invoked.");
        requireEqual(102ULL, levels.getFirst()->price, "Compaction order is wrong.");
    });
}
