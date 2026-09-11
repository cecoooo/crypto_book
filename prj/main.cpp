#include <iostream>
#include "engine.hpp"
#include "book_container.hpp"
#include <algorithm>
#include <chrono>

int main()
{
    auto start = std::chrono::high_resolution_clock::now();
    BookContainer container(2);
    Engine reader(container);
    reader.loadSnapshot("long_input/lite_snapshot.txt");
    auto stopAfterLoadSnapshot = std::chrono::high_resolution_clock::now();
    std::cout << "Execution time (in seconds):" << std::chrono::duration_cast<std::chrono::microseconds>(stopAfterLoadSnapshot - start).count()/1000000.0 << std::endl;

    reader.loadUpdates("long_input/lite_updates.txt");

    std::cout << "Best Bid:" << container.getBook("BNBBTC").bestBid()->price << std::endl;
    std::cout << "Best Ask:" << container.getBook("BNBBTC").bestAsk()->price << std::endl;

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Execution time (in seconds):" << duration.count()/1000000.0 << std::endl;

    return 0;
}