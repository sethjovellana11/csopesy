#pragma once
#include "ICommand.h"
#include "Process.h"
#include <iostream>
#include <sstream>

class PrintCommand : public ICommand {
    std::string varName;

public:
    PrintCommand(const std::string& varName) : varName(varName) {}

    void execute(Process& process) override {
        const auto& vars = process.getVariables();
        std::string output;

        if (vars.count(varName)) {
            output = varName + " = " + std::to_string(vars.at(varName));
        } else {
            output = "Variable '" + varName + "' is undefined";
        }

        std::ostringstream oss;
        oss << "(" << ScreenInfo::getCurrentTimestamp()
            << ") Core:" << process.getScreenInfo().getCoreID()
            << " \"" << output << "\"";

        process.addLog(oss.str());
    }

    std::string toString() const override {
        return "PRINT " + varName;
    }
};