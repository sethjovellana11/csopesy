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

    // New methods to support process-smi
    void addLog(const std::string& log_message);
    std::vector<std::string> getLogs();
    int getTotalInstructions() const;
    std::unordered_map<std::string, int32_t>& getVariables();

    // Demand Paging
    int getCurrentPage() const;                         

    void incrementCurrentPage();
    void setPagesRequired(int pages) { pagesRequired = pages; }
    int getPagesRequired() const { return pagesRequired; }

    void setMemory(int mem) { memory = mem; memPerPage = static_cast<double>(memory) / pagesRequired; }
    int getMemory() const { return memory; }
    int getMemPerPage() const { return memPerPage; }

private:
    int id;
    int instructionCount;
    int coreID;
    int pagesRequired;
    int currentPage;
    int memory;
    int memPerPage;
    int delayPerInstruction = 50;
    bool memoryAllocated = false;
    
    ScreenInfo screenInfo;
    std::vector<std::shared_ptr<ICommand>> instructions;
    std::unordered_map<std::string, int32_t> variables;

    std::vector<std::string> print_logs;
    mutable std::mutex log_mutex;
};
