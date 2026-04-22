
#pragma once

#include "TradeProcessor.h"
#include <iostream>
#include <thread>
#include <cstdlib>
#include "Logger.h"
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
        Logger::getInstance().info("Trade "+ to_string(t.id)+ " added successfully\n");
        cout << "Trade added successfully.\n";
    }
};

//View All Trades
class ViewAllTradesCommand : public Command {
    TradeProcessor& processor;

public:
    explicit ViewAllTradesCommand(TradeProcessor& p) : processor(p) {}

    void execute() override {
        processor.showTrades();
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
       Logger::getInstance().info("Starting trade processing...\n");
        thread t1(&TradeProcessor::validateTrades, &processor);
        thread t2(&TradeProcessor::processTrades, &processor);
        thread t3(&TradeProcessor::settleTrades, &processor);

        t1.join();
        t2.join();
        t3.join();

        cout << "Processing completed.\n";
        Logger::getInstance().info("Processing completed.\n");
        //processor.showTrades();
    }
};

//View Trades by Status
class ViewTradesByStatusCommand : public Command {
    TradeProcessor& processor;

public:
    explicit ViewTradesByStatusCommand(TradeProcessor& p) : processor(p) {}

    void execute() override {
        int s;
        std::cout << "Select Status:\n"
                  << "1.NEW 2.VALIDATED 3.PROCESSED 4.SETTLED 5.REJECTED\n";
        

          Logger::getInstance().info(
        "Select Status:\n"
        "1.NEW 2.VALIDATED 3.PROCESSED 4.SETTLED 5.REJECTED"
    );

        std::cin >> s;

        if (s < 1 || s > 5) {
            std::cout << "Invalid status.\n";
            Logger::getInstance().error("Invalid status. \n");
            return;
        }

        processor.showTradesByStatus(static_cast<Status>(s - 1));
    }
};

//Delete Trade
class DeleteTradeCommand : public Command {
    TradeProcessor& processor;

public:
    explicit DeleteTradeCommand(TradeProcessor& p) : processor(p) {}

    void execute() override {
        int id;
        std::cout << "Enter Trade ID to delete: ";
        
        std::cin >> id;

        if (processor.deleteTrade(id))
           {std::cout << "Trade deleted.\n";
            Logger::getInstance().info("Deleted Trade :"+to_string(id));}
        else
            {std::cout << "Trade not found.\n";
             Logger::getInstance().info("Trade not found.\n");}
    }
};

//Show Metrics
class ShowMetricsCommand : public Command {
    TradeProcessor& processor;

public:
    explicit ShowMetricsCommand(TradeProcessor& p) : processor(p) {}

    void execute() override {
        processor.showMetrics();
    }
};

//Reset System
class ResetSystemCommand : public Command {
    TradeProcessor& processor;

public:
    explicit ResetSystemCommand(TradeProcessor& p) : processor(p) {}

    void execute() override {
        processor.reset();
        std::cout << "System reset completed.\n";
        Logger::getInstance().info("System reset completed.\n");
    }
};

//Exit
class ExitCommand : public Command {
public:
    void execute() override {
        cout << "Exiting system. Goodbye.\n";
        Logger::getInstance().info("Exiting system. Goodbye.\n\n-------------------------------------------\n");
        exit(0);
    }
};
