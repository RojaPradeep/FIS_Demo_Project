#include "TradeProcessor.h"
#include "TradeCLI.h"

#include <iostream>
#include <thread>
#include <unordered_map>
#include <memory>
#include <optional>

namespace
{ 
    constexpr int ADD_TRADE             = 1;
    constexpr int VIEW_ALL_TRADES       = 2;
    constexpr int START_PROCESSING      = 3;
    constexpr int VIEW_BY_STATUS        = 4;
    constexpr int DELETE_TRADE          = 5;
    constexpr int SHOW_METRICS          = 6;
    constexpr int RESET_SYSTEM          = 7;
    constexpr int EXIT_APP              = 8;
    using CommandMap = std::unordered_map<int, std::unique_ptr<Command>>;
}

// Display menu
void showMenu()
{
    std::cout << "\n=== Trade Processing & Settlement Engine ===\n"
              << "1. Add Trade\n"
              << "2. View All Trades\n"
              << "3. Start Processing\n"
              << "4. View Trades by Status\n"
              << "5. Delete Trade\n"
              << "6. Show Metrics / Stats\n"
              << "7. Reset System\n"
              << "8. Exit\n"
              << "Enter your choice: ";
}

std::optional<int> readChoice()
{
    int value{};
    if (std::cin >> value)
        return value;
    return std::nullopt;
}

int main()
{
    TradeProcessor processor;
    CommandMap commands;
    
    commands.emplace(ADD_TRADE,
        std::make_unique<AddTradeCommand>(processor));

    commands.emplace(VIEW_ALL_TRADES,
        std::make_unique<ViewAllTradesCommand>(processor));

    commands.emplace(START_PROCESSING,
        std::make_unique<StartProcessingCommand>(processor));

    commands.emplace(VIEW_BY_STATUS,
        std::make_unique<ViewTradesByStatusCommand>(processor));

    commands.emplace(DELETE_TRADE,
        std::make_unique<DeleteTradeCommand>(processor));

    commands.emplace(SHOW_METRICS,
        std::make_unique<ShowMetricsCommand>(processor));

    commands.emplace(RESET_SYSTEM,
        std::make_unique<ResetSystemCommand>(processor));

    commands.emplace(EXIT_APP,
        std::make_unique<ExitCommand>());

    while (true)
    {
        showMenu();
        
        if (auto choice = readChoice(); choice && commands.count(*choice))
            commands[*choice]->execute();
        else
        {
            std::cout << "Invalid choice. Please try again.\n";
            break;
        }
    }
}