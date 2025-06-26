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
        // The process parameter is unused here, but required by the interface.
        std::this_thread::sleep_for(std::chrono::milliseconds(ticks * 100));
    }

    std::string toString() const override {
    return "SLEEP for " + std::to_string(ticks) + " ticks";
    }
};