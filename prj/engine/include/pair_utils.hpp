#pragma once
#include <iostream>
#include <string_view>
#include <cstdint>

std::uint64_t parseFixedPoint(std::string_view str);
bool extractNextPair(const char*& ptr, const char*& fileEnd, std::string_view& price, std::string_view& qty);
