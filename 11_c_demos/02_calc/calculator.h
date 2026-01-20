#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <string>
#include <stack>
#include <vector>

class Calculator {
public:
    double evaluate(const std::string& expression);
    
private:
    std::vector<std::string> tokenize(const std::string& expression);
    double applyOperator(double a, double b, const std::string& op);
    int getPrecedence(const std::string& op);
    bool isOperator(const std::string& token);
    bool isNumber(const std::string& token);
};

#endif // CALCULATOR_H