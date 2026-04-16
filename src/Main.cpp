#include "TradeProcessor.h"
#include "TradeCLI.h"

#include <iostream>
#include <thread>
#include <unordered_map>
#include <memory>
#include <optional>

namespace
{
    constexpr int ADD_TRADE = 1;
    constexpr int START_PROCESSING = 2;
    constexpr int EXIT_APP = 3;
    using CommandMap = std::unordered_map<int, std::unique_ptr<Command>>;
}

// Display menu
void showMenu()
{
    std::cout << "\n=== Trade Processing & Settlement Engine ===\n"
              << "1. Add Trade\n"
              << "2. Start Processing\n"
              << "3. Exit\n"
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

    commands.emplace(START_PROCESSING,
        std::make_unique<StartProcessingCommand>(processor));

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