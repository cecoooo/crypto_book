#pragma once
#include <iostream>
#include <string_view>
#include <cstdint>

constexpr uint64_t PRICE_LEVEL_MULTIPLIER = 1000000000ULL;
constexpr uint32_t BUFFER_MAX_SIZE = 131072U;
constexpr std::string_view ORDER_ASC = "ASC";
constexpr std::string_view ORDER_DESC = "DESC";

std::uint64_t parseFixedPoint(std::string_view str);
bool extractNextPair(const char*& ptr, const char*& fileEnd, std::string_view& price, std::string_view& qty);
std::string fixedPointToString(std::uint64_t value);