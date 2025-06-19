#pragma once
#include "ICommand.h"

class DeclareCommand : public ICommand {
    std::string var;
    int32_t value;
public:
    DeclareCommand(const std::string& var, int32_t value) : var(var), value(value) {}

    void execute(std::unordered_map<std::string, int32_t>& vars) override {
        vars[var] = static_cast<int32_t>(value);
    }

    std::string toString() const override{
    return "DECLARE " + var + " = " + std::to_string(value);
    }
};

