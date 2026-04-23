#include "TradeProcessor.h"
#include "TradeStrategy.h"
#include "Logger.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <memory>
#include <algorithm>
#include <string>
#include <sstream>
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
        Logger::getInstance().info("Validating trades...\n");
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
    Logger::getInstance().info("Processing trades...\n");
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
    Logger::getInstance().info("Settling trades...\n");
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
    std::scoped_lock lock(mtx);

    std::cout << "\nFinal Trade States:\n";
    Logger::getInstance().info("Final Trade States:\n");

    std::string logMsg;
    logMsg.reserve(trades.size() * 64); // optional: reduce reallocations

    for (const auto& t : trades)
    {
        std::cout << "Trade " << t.id << " | " << t.type << " | " << t.amount << " | ";

        // build one line
        std::string line = std::to_string(t.id)+ " | "+ t.type +" | "+ std::to_string(t.amount)+" | ";

        switch (t.status)
        {
            case Status::NEW:       std::cout << "NEW";       line += "NEW";       break;
            case Status::VALIDATED: std::cout << "VALIDATED"; line += "VALIDATED"; break;
            case Status::PROCESSED: std::cout << "PROCESSED"; line += "PROCESSED"; break;
            case Status::SETTLED:   std::cout << "SETTLED";   line += "SETTLED";   break;
            case Status::REJECTED:  std::cout << "REJECTED";  line += "REJECTED";  break;
        }

        std::cout << '\n';
        line += '\n';

        logMsg += line;               //  accumulate all trades
    }

    Logger::getInstance().info(logMsg); //  logs ALL trades
}

//Display Trades by Status
void TradeProcessor::showTradesByStatus(Status status) const
{
    string strtype=toString(status);
    scoped_lock lock(mtx);

    cout << "\nTrades with Status: "
         << toString(status)
         << "\n-----------------------------------\n";

    string logMsg= "Trades with Status:  " + strtype;
    Logger::getInstance().info(logMsg);
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
            
            logMsg="TradeID : " +to_string(trade.id)+" Trade Type :"+trade.type+" Amount :"+to_string(trade.amount);
            Logger::getInstance().info(logMsg);
        }
    }

    if (!found)
    {
        std::cout << "No trades found for the selected status.\n";
        logMsg= "No trades found for the selected status.\n";
        Logger::getInstance().warn(logMsg);
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

   
// Build a single formatted log message
    std::ostringstream oss;
    oss << "\n========== Trade Metrics ==========\n"
        << "Total Trades       : " << totalTrades      << '\n'
        << "NEW                : " << newCount         << '\n'
        << "VALIDATED          : " << validated   << '\n'
        << "PROCESSED          : " << processed   << '\n'
        << "SETTLED            : " << settled     << '\n'
        << "REJECTED           : " << rejected    << '\n'
        << "===================================\n";

    const std::string logMsg = oss.str();

    // Print to console (like your existing style)
    std::cout << logMsg;

    // Log once (cleaner than logging line-by-line)
    Logger::getInstance().info(logMsg);

}

//System reset
void TradeProcessor::reset()
{
    std::scoped_lock lock(mtx);
    string logMsg{};
    trades.clear();

    validated = false;
    processed = false;
    
    validatedqueue =false;
    processedqueue=false;

    validatedQueue.reset();
    processedQueue.reset();


    logMsg="All trades have been cleared. System reset successful.\n";
    std::cout << logMsg;
    Logger::getInstance().info(logMsg);

}

void TradeProcessor::updateTradeStatusById(int tradeId,Status status)
{
  //  std::scoped_lock lock(mtx);
    for(auto& t:trades)
    {
        if(t.id == tradeId)
        {
            t.status = status;
            return;
        }
    }
}

void TradeProcessor::producerValidationStage()
{
     std::scoped_lock sl(mtx);
    Logger::getInstance().info("Producer thread started validation stage.\n");
    std::cout<<"Producer started: validating trades...\n";

    constexpr int MAX_TRADE_AMOUNT = 1000000;
    std::vector<Trade> snapshot;
    {
      //  std::scoped_lock lock(mtx);
        snapshot = trades;
    }
    for(auto& t:snapshot)
    {
        if(t.amount <= 0)
        {
            updateTradeStatusById(t.id,Status::REJECTED);
            Logger::getInstance().warn("Trade rejected due to non-positive amount. Trade ID: " + std::to_string(t.id));
            continue;
        }
        if(t.amount > MAX_TRADE_AMOUNT)
        {
            updateTradeStatusById(t.id, Status::REJECTED);
            Logger::getInstance().warn("Trade rejected due to trade limit exceeded. Trade Id: " + std::to_string(t.id));
            continue;
        }
            
        if(t.type != "BUY" && t.type !="buy" && t.type != "SELL" && t.type !="sell" && t.type != "HOLD" && t.type !="hold")
        {
            updateTradeStatusById(t.id, Status::REJECTED);
            Logger::getInstance().warn("Trade rejected due to unsupported type. Trade ID: " + std::to_string(t.id));
            continue;
        }

        auto Strategy = createStrategy(t.type);
        if( Strategy && Strategy ->validate(t))
        {
            t.status = Status::VALIDATED;
            updateTradeStatusById(t.id,Status::VALIDATED);
            Logger::getInstance().info("Trade validated and pushed to validatedQueue. Trade ID: " + std::to_string(t.id));
            validatedQueue.push(t);
        }
        else
        {
            updateTradeStatusById(t.id,Status::REJECTED);
            Logger::getInstance().warn("Trade rejected duiring strategy validation. Trade ID: " + std::to_string(t.id));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    validatedQueue.close();
    Logger::getInstance().info("Producer thread finished validation. \n");
    std::cout<<"Producer finished validation. \n";
    validatedqueue=true;
    cv.notify_all();
}

void TradeProcessor::ConsumerProcessingStage()
{
    unique_lock<mutex> uq(mtx);
    cv.wait(uq,[this]{return validatedqueue;});
    Logger::getInstance().info("Consumer-1 thread started: processing stage.\n");
    std::cout << "Consumer-1 stared: procesing validated trades ...\n";
    Trade t{};
    while(validatedQueue.pop(t))
    {
        auto strategy = createStrategy(t.type);
        if(strategy)
        {
            strategy->process(t);
            updateTradeStatusById(t.id,Status::PROCESSED);
            Logger::getInstance().info("Trade processed and pushed to processedQueue. Trade ID: " + std::to_string(t.id));
            processedQueue.push(t);
        }
        else
        {
            updateTradeStatusById(t.id, Status::REJECTED);
            Logger::getInstance().error("Strategy creation failed during processing. Trade ID: " + std::to_string(t.id));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    processedQueue.close();
    processedqueue=true;
    uq.unlock();
    cv.notify_all();
    Logger::getInstance().info("Consumer-1 finished processing. \n");
    std::cout<<"Consumer-1 finished processing.\n";
}

void TradeProcessor::consumerSettlementStage()
{
    unique_lock<mutex>uniq(mtx);
    cv.wait(uniq,[this]{return processedqueue;});
    Logger::getInstance().info("Consumer-2 thread started: settlemet stage.\n");
    std::cout<<"Consumer-2 started settling processed trades ...\n";
    Trade t{};
    while(processedQueue.pop(t))
    {
        if(t.status == Status::PROCESSED)
        {
            t.status =Status::SETTLED;
            updateTradeStatusById(t.id,Status::SETTLED);
            Logger::getInstance().info("Trade settled successfully. Trade ID: " + std::to_string(t.id));
            std::cout<< "Settled Trade ID: " << t.id <<"\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    uniq.unlock();
 
    Logger::getInstance().info("Consumer-2 finished settlement.\n");
    std::cout << " Consumer-2 finished settlement.\n";
}

void TradeProcessor::startProducerConsumer()
{
    Logger::getInstance().info("Starting Producer-Consumer module...\n");
    std::cout << "Starting Producer-Consumer module... \n";
    validatedQueue.reset();
    processedQueue.reset();

    std::thread producer(&TradeProcessor::producerValidationStage,this);
    std::thread consumer1(&TradeProcessor::ConsumerProcessingStage,this);
    std::thread consumer2(&TradeProcessor::consumerSettlementStage,this);
    producer.join();
    consumer1.join();
    consumer2.join();
    Logger::getInstance().info("Producer-Consumer module completed.\n");
    std::cout << "Producer-Consumer module completed. \n";
}