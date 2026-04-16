#include "TradeProcessor.h"
#include <thread>
#include <array>

int main()
{
    TradeProcessor processor;

    processor.addTrade({1, "BUY", 1000});
    processor.addTrade({2, "SELL", -50});
    processor.addTrade({3, "BUY", 500});

    // C++17: store threads in std::array
    std::array<std::thread, 3> threads{
        std::thread{[&] { processor.validateTrades(); }},
        std::thread{[&] { processor.processTrades(); }},
        std::thread{[&] { processor.settleTrades(); }}
    };

    // C++17 range-based loop
    for (auto& t : threads)
        t.join();

    processor.showTrades();
    return 0;
}