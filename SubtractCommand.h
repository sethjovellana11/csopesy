#pragma once
#include "ICommand.h"

class SubtractCommand : public ICommand {
    std::string result;
    std::string target;
    std::string op1;
    std::string op2;

public:
    SubtractCommand(const std::string& target, const std::string& op1, const std::string& op2)
        : target(target), op1(op1), op2(op2.empty() ? op1 : op2) {}

    void execute(std::unordered_map<std::string, int32_t>& vars) override {
        int32_t val1 = vars.count(op1) ? vars[op1] : 0;
        int32_t val2 = vars.count(op2) ? vars[op2] : 0;

        vars[target] = val1 - val2;
        result = std::to_string(val1 - val2);
    }

    std::string toString() const override {
        return "Subtract " + target + " = " + op1 + " - " + op2 + " -> " + result;
    }
};