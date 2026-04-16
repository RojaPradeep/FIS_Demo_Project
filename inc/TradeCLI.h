
#pragma once

#include "TradeProcessor.h"
#include <iostream>
#include <thread>
#include <cstdlib>

using namespace std;

//Command class
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
};

//Add Trade
class AddTradeCommand : public Command {
    TradeProcessor& processor;

public:
    explicit AddTradeCommand(TradeProcessor& p)
        : processor(p) {}

    void execute() override {
        Trade t{};

        cout << "Enter Trade ID: ";
        cin >> t.id;

        cout << "Enter Trade Type (BUY / SELL / HOLD): ";
        cin >> t.type;

        cout << "Enter Trade Amount: ";
        cin >> t.amount;

        t.status = Status::NEW;
        processor.addTrade(t);

        cout << "Trade added successfully.\n";
    }
};

//Start Processing 
class StartProcessingCommand : public Command {
    TradeProcessor& processor;

public:
    explicit StartProcessingCommand(TradeProcessor& p)
        : processor(p) {}

    void execute() override {
        cout << "Starting trade processing...\n";

        thread t1(&TradeProcessor::validateTrades, &processor);
        thread t2(&TradeProcessor::processTrades, &processor);
        thread t3(&TradeProcessor::settleTrades, &processor);

        t1.join();
        t2.join();
        t3.join();

        cout << "Processing completed.\n";
        processor.showTrades();
    }
};

//Exit
class ExitCommand : public Command {
public:
    void execute() override {
        cout << "Exiting system. Goodbye.\n";
        exit(0);
    }
};
