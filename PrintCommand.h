#pragma once
#include "ICommand.h"
#include "Process.h"
#include <iostream>
#include <sstream>

class PrintCommand : public ICommand {
    std::string msg;
public:
    PrintCommand(const std::string& message) : msg(message) {}

    void execute(Process& process) override {
        // Formats the log message to match the required output style.
        std::string timestamp = ScreenInfo::getCurrentTimestamp();
        int coreID = process.getScreenInfo().getCoreID();
        std::string message = "Hello world from " + process.getScreenInfo().getName() + "!";
        
        std::ostringstream oss;
        oss << "(" << timestamp << ") Core:" << coreID << " \"" << message << "\"";

        // Adds the formatted log string to the process's internal log list.
        process.addLog(oss.str());
    }

    std::string toString() const override{
        return "PRINT \"Hello world from ...\"";
    }
};