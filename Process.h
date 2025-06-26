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
    void addInstruction(std::shared_ptr<ICommand> instr);
    void executeNextInstruction();
    bool isComplete() const;
    int getID() const;
    ScreenInfo& getScreenInfo();
    void updateScreenInfo();

    // New methods to support process-smi
    void addLog(const std::string& log_message);
    std::vector<std::string> getLogs();
    int getTotalInstructions() const;
    std::unordered_map<std::string, int32_t>& getVariables();

private:
    int id;
    int instructionCount;
    int coreID;
    ScreenInfo screenInfo;
    std::vector<std::shared_ptr<ICommand>> instructions;
    std::unordered_map<std::string, int32_t> variables;

    // A thread-safe buffer to store logs from PRINT commands.
    std::vector<std::string> print_logs;
    mutable std::mutex log_mutex;
};
