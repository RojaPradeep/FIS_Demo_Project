#pragma once
#include "Trade.h"
#include <string>

//Base Startegy
class TradeStrategy{
public:
    virtual bool validate(const Trade& t)=0;
    virtual void process(Trade& t)=0;
    virtual ~TradeStrategy() = default;
};

//BUY Strategy
class BuyStrategy : public TradeStrategy{
public:
    bool validate(const Trade& t) override;
    void process(Trade& t) override;
};

//SELL Strategy
class SellStrategy : public TradeStrategy{
public:
    bool validate(const Trade& t) override;
    void process(Trade& t) override;
};

//HOLD Strategy
class HoldStrategy : public TradeStrategy{
    public:
    bool validate(const Trade& t) override;
    void process(Trade& t) override;
};