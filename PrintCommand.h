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
        // Generate the log message in the format specified by the screenshot
        std::string timestamp = ScreenInfo::getCurrentTimestamp();
        int coreID = process.getScreenInfo().getCoreID();
        std::string message = "Hello world from " + process.getScreenInfo().getName() + "!";
        
        std::ostringstream oss;
        oss << "(" << timestamp << ") Core:" << coreID << " \"" << message << "\"";

        // Add the formatted log to the process's internal log buffer
        process.addLog(oss.str());
    }

    std::string toString() const override{
        // Based on your requirements, the printed message is always the same.
        // The 'msg' from the constructor is ignored for now.
        return "PRINT \"Hello world from ...\"";
    }
};