#pragma once
#include "ScreenInfo.h"
#include "ICommand.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <mutex>

class Process {
public:
    Process(const std::string& name, int id);

    void assignCore(int coreID);
    void setDelay(int ms);
    void addInstruction(std::shared_ptr<ICommand> instr);
    void executeNextInstruction();
    
    bool isComplete() const;
    bool getIsAllocated();
    void setIsAllocated(bool isAllocated);
    
    int getID() const;
    ScreenInfo& getScreenInfo();
    void updateScreenInfo();

    // Memory size management
    void setMemorySize(int size);
    int getMemorySize() const;

    // screen-smi logging
    void addLog(const std::string& log_message);
    std::vector<std::string> getLogs();

    // Variables and instruction count
    int getTotalInstructions() const;
    std::unordered_map<std::string, int32_t>& getVariables();

private:
    int id;
    int coreID;
    int delayPerInstruction = 50;
    int instructionCount = 0;
    int memorySizeBytes = 0;
    bool memoryAllocated = false;

    ScreenInfo screenInfo;
    std::vector<std::shared_ptr<ICommand>> instructions;
    std::unordered_map<std::string, int32_t> variables;

    std::vector<std::string> print_logs;
    mutable std::mutex log_mutex;
};
