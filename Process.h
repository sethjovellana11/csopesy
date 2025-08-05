#pragma once
#include "ScreenInfo.h"
#include "ICommand.h"
#include "SymbolTable.h"
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

    void displayShutdown();

    // New methods to support process-smi
    void addLog(const std::string& log_message);
    std::vector<std::string> getLogs();
    int getTotalInstructions() const;
    std::unordered_map<std::string, int32_t>& getVariables();

    // Demand Paging
    int getCurrentPage() const;                         
    std::string getCurrentInstructionName() const;
    void incrementCurrentPage();
    void setPagesRequired(int pages) { pagesRequired = pages; }
    int getPagesRequired() const { return pagesRequired; }

    void setMemory(int mem) { memory = mem; memorySize = mem; memPerPage = static_cast<double>(memory) / pagesRequired; }
    int getMemory() const { return memorySize; }
    int getMemPerPage() const { return memPerPage; }

    //SYMBOL TABLE
    SymbolTable& getSymbolTable() { return symbolTable; }
    const SymbolTable& getSymbolTable() const { return symbolTable; }

    //READ WRITE
    uint16_t readMemory(uint16_t address) const;
    void writeMemory(uint16_t address, uint16_t value);
    bool isTerminated() const;
    void shutdown(const std::string& reason);

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

    std::string shutdownString;
    
    ScreenInfo screenInfo;
    std::vector<std::shared_ptr<ICommand>> instructions;
    std::unordered_map<std::string, int32_t> variables;

    std::vector<std::string> print_logs;
    mutable std::mutex log_mutex;

    //SYMBOL TABLE
    SymbolTable symbolTable;

    // READ WRITE
    int memorySize = 0; // Total memory allocated to this process in bytes
    std::unordered_map<uint16_t, uint16_t> emulatedMemory; // address → value map
    bool terminated = false;
};
