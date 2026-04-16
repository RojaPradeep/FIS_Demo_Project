#pragma once
#include "Trade.h"
#include "TradeStrategy.h"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <string>

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

    std::unique_ptr<TradeStrategy> createStrategy(const std::string& type);
};

