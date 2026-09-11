#pragma once
#include <cstddef>
#include "price_level.hpp"
#include "price_level_container.hpp"

class Book
{
private:
    PriceLevelContainer bids;
    PriceLevelContainer asks;

public:
    explicit Book(std::size_t expectedLevels = 0);

    void updateBid(uint64_t price, uint64_t quantity);
    void updateAsk(uint64_t price, uint64_t quantity);

    const PriceLevel* bestBid() const noexcept;
    const PriceLevel* bestAsk() const noexcept;

    const PriceLevelContainer& Bids() const noexcept;
    const PriceLevelContainer& Asks() const noexcept;

    void loadInitialBids(std::vector<PriceLevel>&& bidsLevels);
    void loadInitialAsks(std::vector<PriceLevel>&& asksLevels);

    PriceLevelContainer& Bids();
    PriceLevelContainer& Asks();
};