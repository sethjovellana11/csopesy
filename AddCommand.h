#pragma once
#include "ICommand.h"
#include "Process.h"

class AddCommand : public ICommand {
    std::string result;
    std::string target;
    std::string op1;
    std::string op2;

public:
    AddCommand(const std::string& target, const std::string& op1, const std::string& op2)
        : target(target), op1(op1), op2(op2.empty() ? op1 : op2) {}

    void execute(Process& process) override {
        auto& vars = process.getVariables();
        int32_t val1 = vars.count(op1) ? vars[op1] : 0;
        int32_t val2 = vars.count(op2) ? vars[op2] : 0;

        vars[target] = val1 + val2;
        result = std::to_string(val1 + val2);
    }

    std::string toString() const override {
        return "ADD " + target + " = " + op1 + " + " + op2 + " -> " + result;
    }
};