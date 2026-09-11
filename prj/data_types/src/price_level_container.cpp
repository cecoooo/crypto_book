#include "price_level_container.hpp"
#include <stdexcept>
#include <algorithm>

PriceLevelContainer::PriceLevelContainer(){}

PriceLevelContainer::PriceLevelContainer(std::string_view orderWay, std::size_t capacity)
{
    if (orderWay != "DESC" && orderWay != "ASC")
    {
        throw std::invalid_argument(
            "Invalid Order Way! Must be 'ASC' or 'DESC'.");
    }

    orderBy = orderWay;
    data.reserve(capacity);
}

void PriceLevelContainer::addOrUpdateItem(const PriceLevel& item)
{
    if(orderBy == "ASC")
    {
        addOrUpdateAscending(item);
    }
    else
    {
        addOrUpdateDescending(item);
    }
}

void PriceLevelContainer::addOrUpdateAscending(const PriceLevel& item)
{
    overflow_buffer.emplace_back(item);

    if (overflow_buffer.size() >= 131072)
    {
        compact();
    }
}

void PriceLevelContainer::addOrUpdateDescending(const PriceLevel& item)
{
    overflow_buffer.emplace_back(item);

    if (overflow_buffer.size() >= 131072)
    {
        compact();
    }
}

const PriceLevel* PriceLevelContainer::getFirst() const noexcept
{
    if (data.empty())
    {
        return nullptr;
    }

    return &data.front();
}

void PriceLevelContainer::loadInitialLevels(std::vector<PriceLevel>&& levels)
{
    data = std::move(levels);

    if (orderBy == "ASC")
    {
        std::stable_sort(data.begin(), data.end(),
            [](const PriceLevel& a, const PriceLevel& b)
            {
                return a.price < b.price;
            });
    }
    else
    {
        std::stable_sort(data.begin(), data.end(),
            [](const PriceLevel& a, const PriceLevel& b)
            {
                return a.price > b.price;
            });
    }

    std::size_t writeIndex = 0;
    std::size_t readIndex = 0;

    while (readIndex < data.size())
    {
        std::size_t lastIndex = readIndex;

        while (lastIndex + 1 < data.size() &&
               data[lastIndex + 1].price == data[readIndex].price)
        {
            lastIndex++;
        }

        if (data[lastIndex].quantity != 0)
        {
            data[writeIndex++] = data[lastIndex];
        }

        readIndex = lastIndex + 1;
    }

    data.resize(writeIndex);
}

void PriceLevelContainer::compact()
{
    if (overflow_buffer.empty())
    {
        return;
    }

    if (orderBy == "ASC")
    {
        std::stable_sort(overflow_buffer.begin(), overflow_buffer.end(),
            [](const PriceLevel& a, const PriceLevel& b)
            {
                return a.price < b.price;
            });
    }
    else
    {
        std::stable_sort(overflow_buffer.begin(), overflow_buffer.end(),
            [](const PriceLevel& a, const PriceLevel& b)
            {
                return a.price > b.price;
            });
    }

    const std::size_t oldSize = data.size();

    data.insert(data.end(), overflow_buffer.begin(), overflow_buffer.end());

    overflow_buffer.clear();

    if (orderBy == "ASC")
    {
        std::inplace_merge(data.begin(), data.begin() + oldSize, data.end(),
            [](const PriceLevel& a, const PriceLevel& b)
            {
                return a.price < b.price;
            });
    }
    else
    {
        std::inplace_merge(data.begin(), data.begin() + oldSize, data.end(),
            [](const PriceLevel& a, const PriceLevel& b)
            {
                return a.price > b.price;
            });
    }

    std::size_t writeIndex = 0;
    std::size_t readIndex = 0;

    while (readIndex < data.size())
    {
        std::size_t lastIndex = readIndex;

        while (lastIndex + 1 < data.size() &&
               data[lastIndex + 1].price == data[readIndex].price)
        {
            lastIndex++;
        }

        if (data[lastIndex].quantity != 0)
        {
            data[writeIndex++] = data[lastIndex];
        }

        readIndex = lastIndex + 1;
    }

    data.resize(writeIndex);
}