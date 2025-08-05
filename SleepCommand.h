#pragma once
#include "ICommand.h"
#include "Process.h"
#include <thread>
#include <chrono>

class SleepCommand : public ICommand {
    uint8_t ticks;

public:
    SleepCommand(uint8_t ticks) : ticks(ticks) {}

    void execute(Process& process) override {
        std::this_thread::sleep_for(std::chrono::milliseconds(ticks * 100));
    }

    std::string toString() const override {
    return "SLEEP for " + std::to_string(ticks) + " ticks";
    }

    std::string getName() const override {
        return "SLEEP";
    }
};