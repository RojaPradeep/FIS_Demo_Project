#include "TradeProcessor.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <string_view>
#include <algorithm>

using namespace std;
using namespace std::chrono_literals; //  C++17: duration literals

namespace
{
    // C++17: constexpr function + string_view
    constexpr string_view statusToString(Status status) noexcept
    {
        switch (status)
        {
            case Status::NEW:       return "NEW"sv;
            case Status::VALIDATED: return "VALIDATED"sv;
            case Status::PROCESSED: return "PROCESSED"sv;
            case Status::SETTLED:   return "SETTLED"sv;
            case Status::REJECTED:  return "REJECTED"sv;
        }
        return "UNKNOWN"sv;
    }
}

// Add trade
void TradeProcessor::addTrade(const Trade& t)
{
    scoped_lock lock(mtx); // C++17: scoped_lock
    trades.push_back(t);
}

// Thread 1: Validation
void TradeProcessor::validateTrades()
{
    {
        scoped_lock lock(mtx); // C++17
        cout << "Validating trades...\n";

        for (auto& t : trades)
        {
            // Rule 1: Trade must be NEW
            if (t.status != Status::NEW)
            {
                t.status = Status::REJECTED;
                continue;
            }

            // Rule 2: Trade amount must be positive
            if (t.amount <= 0)
            {
                t.status = Status::REJECTED;
                continue;
            }

            // Rule 3: Maximum trade size limit
            constexpr int MAX_TRADE_AMOUNT = 1'000'000; // C++17 digit separator
            if (t.amount > MAX_TRADE_AMOUNT)
            {
                t.status = Status::REJECTED;
                continue;
            }

            // Rule 4: Supported trade types only
            if (t.type != "BUY" && t.type != "SELL")
            {
                t.status = Status::REJECTED;
                continue;
            }

            // Rule 5: SELL trade additional constraint
            if (t.type == "SELL" && t.amount < 10)
            {
                // Minimum sell quantity
                t.status = Status::REJECTED;
                continue;
            }

           

            // If all rules pass
            t.status = Status::VALIDATED;
        }

        validated = true;
    }

    cv.notify_all();
    this_thread::sleep_for(1s); // C++17 chrono literal
}

// Thread 2: Processing
void TradeProcessor::processTrades()
{
    unique_lock lock(mtx); // C++17 deduction guide
    cv.wait(lock, [this] { return validated; });

    cout << "Processing trades...\n";

    for (auto& t : trades)
    {
        if (t.status == Status::VALIDATED)
        {
            t.status = Status::PROCESSED;
        }
    }

    processed = true;
    lock.unlock();

    cv.notify_all();
    this_thread::sleep_for(1s);
}

// Thread 3: Settlement
void TradeProcessor::settleTrades()
{
    unique_lock lock(mtx); //  C++17 deduction guide
    cv.wait(lock, [this] { return processed; });

    cout << "Settling trades...\n";

    for (auto& t : trades)
    {
        if (t.status == Status::PROCESSED)
        {
            t.status = Status::SETTLED;
        }
    }

    this_thread::sleep_for(1s);
}

// Display results
void TradeProcessor::showTrades() const
{
    scoped_lock lock(mtx); // C++17

    cout << "\nFinal Trade States:\n";

    for (const auto& t : trades)
    {
        //  C++17: structured bindings (via tuple-like expansion)
        const auto& [id, type, amount, status] = t;

        cout << "Trade " << id << " | "
             << type << " | "
             << amount << " | "
             << statusToString(status)
             << '\n';
    }
}
