#pragma once
#include "ICommand.h"
#include "Process.h"
#include <sstream>
#include <iostream>

class DeclareCommand : public ICommand {
    std::string var;
    int32_t value;
public:
    DeclareCommand(const std::string& var, int32_t value) : var(var), value(value) {}

    void execute(Process& process) override {
        process.getVariables()[var] = static_cast<int32_t>(value);

        std::string timestamp = ScreenInfo::getCurrentTimestamp();
        int coreID = process.getScreenInfo().getCoreID();

        std::ostringstream oss;
        oss << "(" << timestamp << ") Core:" << coreID << " \"" << "DECLARE " + var + " = " + std::to_string(value) << "\"";
        
        process.addLog(oss.str());
    }

    std::string toString() const override{
    return "DECLARE " + var + " = " + std::to_string(value);
    }
};