#pragma once
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

class TestRunner
{
private:
    std::size_t passed=0;
    std::size_t failed=0;
public:
    template<typename Test>
    void run(const std::string& name, Test test)
    {
        try
        {
            test();
            passed++;
            std::cout << "[PASS] " << name << '\n';
        }
        catch (const std::exception& error)
        {
            failed++;
            std::cout << "[FAIL] " << name << ": " << error.what() << '\n';
        }
        catch (...)
        {
            failed++;
            std::cout << "[FAIL] " << name << ": Unknown exception\n";
        }
    }

    int finish() const
    {
        std::cout
            << "\nTests passed: " << passed << '\n'
            << "Tests failed: " << failed << '\n';

        return failed == 0 ? 0 : 1;
    }
};

inline void require(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

template<typename Expected, typename Actual>
void requireEqual(
    const Expected& expected,
    const Actual& actual,
    const std::string& message)
{
    if (!(expected == actual))
    {
        std::ostringstream stream;
        stream
            << message
            << " Expected: " << expected
            << ", actual: " << actual;
        throw std::runtime_error(stream.str());
    }
}
