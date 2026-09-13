#include "test_suites.hpp"
#include "pair_utils.hpp"
#include <string>
#include <string_view>

void runPairUtilsTests(TestRunner& runner)
{
    runner.run("parse fixed point values", []
    {
        requireEqual(
            12340000000ULL,
            parseFixedPoint("12.34"),
            "Incorrect fixed-point value.");
        requireEqual(
            2398500ULL,
            parseFixedPoint("0.002398500"),
            "Incorrect price value.");
        requireEqual(
            7000000000ULL,
            parseFixedPoint("7"),
            "Incorrect integer value.");
    });

    runner.run("format fixed point values", []
    {
        requireEqual(
            std::string("12.340000000"),
            fixedPointToString(12340000000ULL),
            "Incorrect fixed-point string.");
        requireEqual(
            std::string("0.002398500"),
            fixedPointToString(2398500ULL),
            "Incorrect price string.");
    });

    runner.run("extract price and quantity pair", []
    {
        const std::string data = "[\"1.25\",\"3.50\"]";
        const char* current = data.data();
        const char* end = current + data.size();
        std::string_view price;
        std::string_view quantity;

        require(
            extractNextPair(current, end, price, quantity),
            "Pair was not extracted.");
        requireEqual(
            std::string_view("1.25"),
            price,
            "Incorrect extracted price.");
        requireEqual(
            std::string_view("3.50"),
            quantity,
            "Incorrect extracted quantity.");
    });
}
