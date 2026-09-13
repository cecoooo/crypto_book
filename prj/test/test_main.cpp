#include "test_suites.hpp"
#include <iostream>

int main()
{
    TestRunner runner;

    std::cout << "Order book tests\n\n";

    runPairUtilsTests(runner);
    runPriceLevelContainerTests(runner);
    runBookTests(runner);
    runBookContainerTests(runner);
    runEngineTests(runner);
    runBoundaryTests(runner);
    runInvalidInputTests(runner);

    return runner.finish();
}