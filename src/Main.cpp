#include "TradeProcessor.h"
#include "TradeCLI.h"

#include <iostream>
#include <thread>
#include <unordered_map>
#include <memory>

using namespace std;

// Display menu
void showMenu()
{
    std::cout << "\n=== Trade Processing & Settlement Engine ===\n";
    std::cout << "1. Add Trade\n";
    std::cout << "2. Start Processing\n";
    std::cout << "3. Exit\n";
    std::cout << "Enter your choice: ";
}

int main()
{
    TradeProcessor processor;

    unordered_map<int, unique_ptr<Command>> commands;

    commands.emplace(1, make_unique<AddTradeCommand>(processor));
    commands.emplace(2, make_unique<StartProcessingCommand>(processor));
    commands.emplace(3, make_unique<ExitCommand>());

    while (true)
    {
        showMenu();

        int choice{};
        cin >> choice;

        auto it = commands.find(choice);
        if (it != commands.end()) {
            it->second->execute();
        } else {
            cout << "Invalid choice. Please try again.\n";
            break;
        }
    }
}