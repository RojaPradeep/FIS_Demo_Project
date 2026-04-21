#include "TradeProcessor.h"
#include "TradeStrategy.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <memory>
#include <algorithm>

using namespace std;
unique_ptr<TradeStrategy> TradeProcessor::createStrategy(const std::string& type)
{
    if(type == "BUY")
    {
        return make_unique<BuyStrategy>();
    }
    else if(type == "SELL")
    {
        return make_unique<SellStrategy>();
    }
    else if(type == "HOLD")
    {
        return make_unique<HoldStrategy>();
    }
    return nullptr;
}

// Add trade
void TradeProcessor::addTrade(const Trade& t)
{
    scoped_lock lock(mtx);
    trades.push_back(t);
}

// Thread 1: Validation
void TradeProcessor::validateTrades()
{
    {
        scoped_lock lock(mtx);
        cout << "Validating trades...\n";
        constexpr int MAX_TRADE_AMOUNT = 1'000'000;
        for (auto& t : trades)
        {
            // Amount must be positive
            if(t.amount <= 0)
            {
                t.status = Status::REJECTED;
                continue;
            }
            
            //MAX trade limit
            if(t.amount > MAX_TRADE_AMOUNT)
            {
                t.status = Status::REJECTED;
                continue;
            }
            
            //Supported trade types only
            if(t.type != "BUY" && t.type !="buy" && t.type != "SELL" && t.type !="sell" && t.type != "HOLD" && t.type !="hold")
            {
                t.status = Status::REJECTED;
                continue;
            }

            auto Strategy = createStrategy(t.type);
            if( Strategy && Strategy ->validate(t))
            {
                t.status = Status::VALIDATED;
            }
            else
            {
                t.status = Status::REJECTED;
            }
        }

        validated = true;
    }

    cv.notify_all(); // signal processing stage
    this_thread::sleep_for(1s);
}

// Thread 2: Processing
void TradeProcessor::processTrades()
{
    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [this] { return validated; });

    cout << "Processing trades...\n";

    for (auto& t : trades)
    {
        if (t.status == Status::VALIDATED){
            //t.status = Status::PROCESSED;
            auto strategy = createStrategy(t.type);
            if(strategy)
                strategy->process(t);
        }
    }

    processed = true;
    lock.unlock();

    cv.notify_all(); // signal settlement stage
    this_thread::sleep_for(1s);
}

// Thread 3: Settlement
void TradeProcessor::settleTrades()
{
    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [this] { return processed; });

    cout << "Settling trades...\n";

    for (auto& t : trades)
    {
        if (t.status == Status::PROCESSED)
            t.status = Status::SETTLED;
    }

    lock.unlock();
    this_thread::sleep_for(1s);
}

// Display results
void TradeProcessor::showTrades() const
{
    scoped_lock lock(mtx);

    cout << "\nFinal Trade States:\n";

    for (const auto& t : trades)
    {
        cout << "Trade "
                  << t.id << " | "
                  << t.type << " | "
                  << t.amount << " | ";

        switch (t.status)
        {
            case Status::NEW:       cout << "NEW"; break;
            case Status::VALIDATED: cout << "VALIDATED"; break;
            case Status::PROCESSED: cout << "PROCESSED"; break;
            case Status::SETTLED:   cout << "SETTLED"; break;
            case Status::REJECTED:  cout << "REJECTED"; break;
        }

        cout << '\n';
    }
}

//Display Trades by Status
void TradeProcessor::showTradesByStatus(Status status) const
{
    scoped_lock lock(mtx);

    cout << "\nTrades with Status: "
         << toString(status)
         << "\n-----------------------------------\n";

    bool found{false};

    for (const auto& trade : trades)
    {
        if (trade.status == status)
        {
            cout << "Trade ID   : " << trade.id << '\n'
                 << "Trade Type : " << trade.type << '\n'
                 << "Amount     : " << trade.amount << '\n'
                 << "-----------------------------------\n";
            found = true;
        }
    }

    if (!found)
    {
        std::cout << "No trades found for the selected status.\n";
    }
}

//Delete Trade
bool TradeProcessor::deleteTrade(int tradeId)
{
    scoped_lock lock(mtx);

    const auto it = std::find_if(trades.begin(), trades.end(), [tradeId](const Trade& t) {
            return t.id == tradeId;
        });

    if (it != trades.end())
    {
        trades.erase(it);
        return true;
    }

    return false;
}

//Show Metrics
void TradeProcessor::showMetrics() const
{
    scoped_lock lock(mtx);

    const size_t totalTrades { trades.size() };

    size_t newCount      { 0 };
    size_t validated     { 0 };
    size_t processed     { 0 };
    size_t settled       { 0 };
    size_t rejected      { 0 };

    for (const auto& trade : trades)
    {
        switch (trade.status)
        {
            case Status::NEW:        ++newCount;  break;
            case Status::VALIDATED:  ++validated; break;
            case Status::PROCESSED:  ++processed; break;
            case Status::SETTLED:    ++settled;   break;
            case Status::REJECTED:   ++rejected;  break;
        }
    }

    cout << "\n========== Trade Metrics ==========\n"
         << "Total Trades       : " << totalTrades << '\n'
         << "NEW                : " << newCount    << '\n'
         << "VALIDATED          : " << validated   << '\n'
         << "PROCESSED          : " << processed   << '\n'
         << "SETTLED            : " << settled     << '\n'
         << "REJECTED           : " << rejected    << '\n'
         << "===================================\n";
}

//System reset
void TradeProcessor::reset()
{
    std::scoped_lock lock(mtx);

    trades.clear();

    std::cout << "All trades have been cleared. System reset successful.\n";
}
