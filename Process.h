#pragma once
#include "ScreenInfo.h"
#include <string>

class Process {
public:
    Process(const std::string& name, int id);

    void executeNextInstruction();
    bool isComplete() const;

    int getID() const;
    const ScreenInfo& getScreenInfo() const;
    void updateScreenInfo();

private:
    int id;
    int instructionCount;
    ScreenInfo screenInfo;
};
