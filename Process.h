#pragma once
#include "ScreenInfo.h"
#include "ICommand.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

class Process {
public:
    Process(const std::string& name, int id);

    void assignCore(int coreID);
    void addInstruction(std::shared_ptr<ICommand> instr);
    void executeNextInstruction();
    bool isComplete() const;
    int getID() const;
    const ScreenInfo& getScreenInfo() const;
    void updateScreenInfo();

private:
    int id;
    int instructionCount;
    int coreID;
    ScreenInfo screenInfo;
    std::vector<std::shared_ptr<ICommand>> instructions;
    std::unordered_map<std::string, int32_t> variables;
};