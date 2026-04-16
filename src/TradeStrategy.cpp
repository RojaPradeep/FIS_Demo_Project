#include "TradeStrategy.h"
#include <iostream>

//BUY Strategy
bool BuyStrategy::validate(const Trade& t){
    return t.amount > 0;
}

void BuyStrategy::process(Trade& t){
    std::cout << "Executing BUY trade: " << t.id << std ::endl;
    t.status = Status::PROCESSED;
}

//SELL Strategy
bool SellStrategy::validate(const Trade& t){
    return t.amount > 0;
}

void SellStrategy::process(Trade& t){
    std::cout << "Executing SELL trade: " << t.id << std ::endl;
    t.status = Status::PROCESSED;
}


//HOLD Strategy
bool HoldStrategy::validate(const Trade& t){
    return t.amount >= 0;
}

void HoldStrategy::process(Trade& t){
    std::cout << "Executing HOLD trade: " << t.id << std ::endl;
    t.status = Status::PROCESSED;
}