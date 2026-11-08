#include <iostream>
#include <string>
#include <windows.h>
#include "parser.h"
#include "plugin_manager.h"

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(65001);

    PluginManager manager;

    try {
        manager.LoadPlugins();
    }
    catch (const std::exception& e) {
        std::cerr << "[FATAL] Error loading plugins: " << e.what() << "\n";
        return 1;
    }

    if (argc > 1) {
        std::string expression;
        for (int i = 1; i < argc; i++) {
            if (i > 1) expression += " ";
            expression += argv[i];
        }

        try {
            Parser parser(expression, manager);
            double result = parser.Parse();
            std::cout << result << "\n";
            return 0;
        }
        catch (const std::exception& e) {
            std::cerr << "[ERROR] " << e.what() << "\n";
            return 1;
        }
    }

    std::cout << "Calculator started. Enter expression (or 'exit' to quit):\n";

    std::string input;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, input)) break;
        if (input == "exit") break;
        if (input.empty()) continue;

        try {
            Parser parser(input, manager);
            double result = parser.Parse();
            std::cout << "= " << result << "\n";
        }
        catch (const std::exception& e) {
            std::cerr << "[ERROR] " << e.what() << "\n";
        }
        catch (...) {
            std::cerr << "[ERROR] Unknown exception!\n";
        }
    }

    return 0;
}
