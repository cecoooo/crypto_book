#include <iostream>
#include "engine.hpp"
#include "book_container.hpp"
#include <algorithm>
#include <chrono>
#include "pair_utils.hpp"

int main()
{
    BookContainer container(2);
    Engine reader(container);
    reader.loadSnapshot("long_input/lite_snapshot.txt");
    auto stopAfterLoadSnapshot = std::chrono::high_resolution_clock::now();

    reader.loadUpdates("long_input/lite_updates.txt");

    std::cout << "Best Bid: " << fixedPointToString(container.getBook("BNBBTC").bestBid()->price) << std::endl;
    std::cout << "Best Ask: " << fixedPointToString(container.getBook("BNBBTC").bestAsk()->price) << std::endl;

    return 0;
}