#pragma once
#include "Trade.h"
#include <queue>
#include <mutex>
#include <condition_variable>

class TradeQueue
{
private:
    std::queue<Trade> q;
    mutable std::mutex mtx;
    std::condition_variable cv;
    bool closed{false};

public:
    void push(const Trade& trade);
    bool pop(Trade& trade);
    void close();
    void reset();
    bool empty() const;
};