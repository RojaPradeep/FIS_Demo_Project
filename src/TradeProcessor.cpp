#include "TradeProcessor.h"
#include "TradeStrategy.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <memory>

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
    lock_guard<mutex> lock(mtx);
    trades.push_back(t);
}

// Thread 1: Validation
void TradeProcessor::validateTrades()
{
    {
        lock_guard<mutex> lock(mtx);
        cout << "Validating trades...\n";

        for (auto& t : trades)
        {
            auto Strategy = createStrategy(t.type);
            if( Strategy && Strategy ->validate(t))
            {
                t.status = Status::VALIDATED;
            }
            else
            {
                t.status = Status::REJECTED;
            }
            /*if (t.amount <= 0)
                t.status = Status::REJECTED;
            else
                t.status = Status::VALIDATED;*/
        }

        validated = true;
    }

    cv.notify_all(); // signal processing stage
    this_thread::sleep_for(chrono::seconds(1));
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
    this_thread::sleep_for(chrono::seconds(1));
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
    this_thread::sleep_for(chrono::seconds(1));
}

// Display results
void TradeProcessor::showTrades() const
{
    lock_guard<mutex> lock(mtx);

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