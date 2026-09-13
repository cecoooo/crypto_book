#pragma once
#include <cstdint>

struct PriceLevel
{
    std::uint64_t price;
    std::uint64_t quantity;

    PriceLevel(){}
    PriceLevel(std::uint64_t p, std::uint64_t q): price{p}, quantity{q}{}

    bool operator==(const PriceLevel& other) const noexcept
    {
        return price == other.price &&
               quantity == other.quantity;
    }
};