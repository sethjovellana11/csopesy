#pragma once
#include "ICommand.h"
#include <iostream>

class PrintCommand : public ICommand {
    std::string msg;
public:
    PrintCommand(const std::string& message) : msg(message) {}

    void execute(std::unordered_map<std::string, int32_t>& vars) override {
        msg = "Hello World!";
    }
    std::string toString() const override{
    return msg;
    }
};