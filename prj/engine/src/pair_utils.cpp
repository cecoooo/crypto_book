#include "pair_utils.hpp"

std::uint64_t parseFixedPoint(std::string_view str)
{
    std::uint64_t intPart = 0;
    std::uint64_t fracPart = 0;
    std::size_t i = 0;
    
    while (i < str.size() && str[i] != '.') {
        intPart = intPart * 10 + (str[i] - '0');
        i++;
    }
    
    if (i < str.size() && str[i] == '.') {
        std::size_t fracDigits = 0;
        while (i < str.size() && fracDigits < 8) {
            fracPart = fracPart * 10 + (str[i] - '0');
            fracDigits++;
            i++;
        }
        while (fracDigits < 8) {
            fracPart *= 10;
            fracDigits++;
        }
    }
    
    return (intPart * 100000000) + fracPart;
}

bool extractNextPair(const char*& ptr, const char*& fileEnd, std::string_view& price, std::string_view& qty)
{
    while (ptr < fileEnd && *ptr != '"') ptr++;
    if (ptr >= fileEnd) return false;
    const char* pStart = ++ptr;
    while (ptr < fileEnd && *ptr != '"') ptr++;
    if (ptr >= fileEnd) return false;
    price = std::string_view(pStart, ptr - pStart);
    
    ptr++;
    while (ptr < fileEnd && *ptr != '"') ptr++;
    if (ptr >= fileEnd) return false;
    const char* qStart = ++ptr;
    while (ptr < fileEnd && *ptr != '"') ptr++;
    if (ptr >= fileEnd) return false;
    qty = std::string_view(qStart, ptr - qStart);
    
    ptr++;
    return true;
}