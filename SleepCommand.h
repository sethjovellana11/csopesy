#pragma once
#include "ICommand.h"
#include <thread>
#include <chrono>

class SleepCommand : public ICommand {
    uint8_t ticks;

public:
    SleepCommand(uint8_t ticks) : ticks(ticks) {}

    void execute(std::unordered_map<std::string, int32_t>& vars) override {
        std::this_thread::sleep_for(std::chrono::milliseconds(ticks * 100));
    }

    std::string toString() const override {
    return "SLEEP for " + std::to_string(ticks) + " ticks";
    }
};