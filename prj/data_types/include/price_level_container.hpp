#pragma once
#include <vector>
#include <string_view>
#include "price_level.hpp"
#include "pair_utils.hpp"

class PriceLevelContainer
{
private:
    std::vector<PriceLevel> data;
    std::string_view orderBy=ORDER_ASC;
    std::vector<PriceLevel> overflow_buffer;
    std::size_t bufferMaxSize=BUFFER_MAX_SIZE;
public:
    PriceLevelContainer();
    PriceLevelContainer(std::string_view orderWay, std::size_t capacity, std::size_t maxBufferSize=BUFFER_MAX_SIZE);
    void addOrUpdateItem(const PriceLevel& item);
    const PriceLevel* getFirst() const noexcept;
    void loadInitialLevels(std::vector<PriceLevel>&& levels);
    void compact();
    std::size_t levelsCount() const noexcept;
    std::size_t pendingUpdatesCount() const noexcept;
    std::size_t allocatedBytes() const noexcept;
};
