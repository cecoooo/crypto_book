#pragma once
#include <vector>
#include <string>
#include <string_view>
#include "price_level.hpp"

class PriceLevelContainer
{
private:
    std::vector<PriceLevel> data;
    std::string orderBy="ASC";
    std::vector<PriceLevel> overflow_buffer;
    void addOrUpdateAscending(const PriceLevel& item);
    void addOrUpdateDescending(const PriceLevel& item);
public:
    PriceLevelContainer();
    PriceLevelContainer(std::string_view orderWay, std::size_t capacity);
    void addOrUpdateItem(const PriceLevel& item);
    const PriceLevel* getFirst() const noexcept;
    void loadInitialLevels(std::vector<PriceLevel>&& levels);
    void compact();
};