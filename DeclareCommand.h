#pragma once
#include "ICommand.h"
#include "Process.h"

class DeclareCommand : public ICommand {
    std::string var;
    int32_t value;
public:
    DeclareCommand(const std::string& var, int32_t value) : var(var), value(value) {}

    void execute(Process& process) override {
        process.getVariables()[var] = static_cast<int32_t>(value);
    }

    std::string toString() const override{
    return "DECLARE " + var + " = " + std::to_string(value);
    }
};