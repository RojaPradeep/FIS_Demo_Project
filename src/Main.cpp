#include "TradeProcessor.h"

#include <thread>

using namespace std;

int main()
{
    TradeProcessor processor;

    processor.addTrade({1, "BUY", 1000});
    processor.addTrade({2, "SELL", -50});
    processor.addTrade({3, "BUY", 500});

    thread t1(&TradeProcessor::validateTrades, &processor);
    thread t2(&TradeProcessor::processTrades, &processor);
    thread t3(&TradeProcessor::settleTrades, &processor);

    t1.join();
    t2.join();
    t3.join();

    processor.showTrades();

    return 0;
}