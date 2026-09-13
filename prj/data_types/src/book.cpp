#include "book.hpp"
#include "pair_utils.hpp"

Book::Book(std::size_t expectedLevels, std::size_t bufferMaxSize): 
    bids{PriceLevelContainer(ORDER_DESC, expectedLevels, bufferMaxSize)}, 
    asks{PriceLevelContainer(ORDER_ASC, expectedLevels, bufferMaxSize)}
{}

const PriceLevelContainer& Book::Bids() const noexcept
{
    return bids;
}

const PriceLevelContainer& Book::Asks() const noexcept
{
    return asks;
}

PriceLevelContainer& Book::Bids()
{
    return bids;
}

PriceLevelContainer& Book::Asks()
{
    return asks;
}

void Book::updateBid(uint64_t price, uint64_t quantity)
{
    bids.addOrUpdateItem(PriceLevel(price, quantity));
}

void Book::updateAsk(uint64_t price, uint64_t quantity)
{
    asks.addOrUpdateItem(PriceLevel(price, quantity));
}

const PriceLevel* Book::bestBid() const noexcept
{
    return bids.getFirst();
}

const PriceLevel* Book::bestAsk() const noexcept
{
    return asks.getFirst();
}

void Book::loadInitialBids(std::vector<PriceLevel>&& bidsLevels) {
    bids.loadInitialLevels(std::move(bidsLevels));
}

void Book::loadInitialAsks(std::vector<PriceLevel>&& asksLevels) {
    asks.loadInitialLevels(std::move(asksLevels));
}
