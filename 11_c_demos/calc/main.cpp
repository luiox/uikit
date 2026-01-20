#include <iostream>
#include <string>
#include <stdexcept>
#include "calculator.h"

int main() {
    Calculator calc;
    std::string expression;
    
    std::cout << "Simple Calculator" << std::endl;
    std::cout << "Enter an expression (or 'quit' to exit): " << std::endl;
    
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, expression);
        
        if (expression == "quit" || expression == "exit") {
            break;
        }
        
        try {
            double result = calc.evaluate(expression);
            std::cout << "Result: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    
    return 0;
}