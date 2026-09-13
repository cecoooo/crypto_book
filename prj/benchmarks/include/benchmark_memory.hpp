#pragma once
#include <cstddef>

struct MemoryUsage
{
    std::size_t residentBytes=0;
    std::size_t peakResidentBytes=0;
    std::size_t privateBytes=0;
};

MemoryUsage getMemoryUsage() noexcept;
