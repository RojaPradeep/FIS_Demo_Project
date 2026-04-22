#include "TradeQueue.h"

void TradeQueue::push(const Trade& trade)
{
    std::lock_guard<std::mutex> lock(mtx);
    if(closed)
        return;
    q.push(trade);
    cv.notify_one();
}

bool TradeQueue::pop(Trade& trade)
{
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock,[this]{return closed || !q.empty();});
    if(q.empty())
        return false;
    
    trade = q.front();
    q.pop();
    return true;
}

void TradeQueue::close()
{
    {
        lock_guard<std::mutex> lock(mtx);
        closed = true;
    }
    cv.notify_all();
}

void TradeQueue::reset()
{
    std::lock_guard<std::mutex> lock(mtx);
    std::queue<Trade> emptyQueue;
    std::swap(q,emptyQueue);
    closed = false;
}
bool TradeQueue::empty() const
{
    std::lock_guard<std::mutex> lock(mtx);
    return q.empty();
}
