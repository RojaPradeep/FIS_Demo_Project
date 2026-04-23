#pragma once
#include "Trade.h"
#include "TradeStrategy.h"
#include "TradeQueue.h"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <string>

using namespace std;

namespace
{
    constexpr const char* toString(Status status) noexcept
    {
        switch (status)
        {
            case Status::NEW:        return "NEW";
            case Status::VALIDATED:  return "VALIDATED";
            case Status::PROCESSED:  return "PROCESSED";
            case Status::SETTLED:    return "SETTLED";
            case Status::REJECTED:   return "REJECTED";
        }
        return "UNKNOWN";
    }
}

class TradeProcessor
{
public:
    void addTrade(const Trade& t);

    void validateTrades();
    void processTrades();
    void settleTrades();

    void showTrades() const;
    void showTradesByStatus(Status) const;
    bool deleteTrade(int);
    void showMetrics() const;
    void reset();
    void startProducerConsumer();
   
private:
    std::vector<Trade> trades;

    mutable std::mutex mtx;
    std::condition_variable cv;

    bool validated{false};
    bool processed{false};
    
    bool validatedqueue{false};
    bool processedqueue{false};
  
    std::unique_ptr<TradeStrategy> createStrategy(const std::string& type);

    TradeQueue validatedQueue;
    TradeQueue processedQueue;

    void producerValidationStage();
    void ConsumerProcessingStage();
    void consumerSettlementStage();
    void updateTradeStatusById(int tradeId, Status status);
    
};

