#include "TradeProcessor.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>

using namespace std;

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
            if (t.amount <= 0)
                t.status = Status::REJECTED;
            else
                t.status = Status::VALIDATED;
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
        if (t.status == Status::VALIDATED)
            t.status = Status::PROCESSED;
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