#pragma once
#include "Trade.h"
#include <vector>
#include <mutex>
#include <condition_variable>

using namespace std;

class TradeProcessor
{
public:
    void addTrade(const Trade& t);

    void validateTrades();
    void processTrades();
    void settleTrades();

    void showTrades() const;

private:
    std::vector<Trade> trades;

    mutable std::mutex mtx;
    std::condition_variable cv;

    bool validated{false};
    bool processed{false};
};

