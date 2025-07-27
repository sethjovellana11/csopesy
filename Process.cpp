#include "Process.h"
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

Process::Process(const std::string& name, int id)
    : id(id), instructionCount(0), coreID(-1), screenInfo(name) {
}

void Process::setDelay(int ms) {
    delayPerInstruction = ms;
}

void Process::setIsAllocated(bool isAlloc){
    memoryAllocated = isAlloc;
}

bool Process:: getIsAllocated(){
    return memoryAllocated;
}

void Process::assignCore(int coreID) {
    this->coreID = coreID;
    screenInfo.setCoreID(coreID);
}
void Process::addInstruction(std::shared_ptr<ICommand> instr) {
    instructions.push_back(instr);
}

// executeNextInstruction now passes a reference to itself to the command.
void Process::executeNextInstruction() {
    // Create logs directory if it doesn't exist.
    if (!std::filesystem::exists("logs")) {
        std::filesystem::create_directory("logs");
    }

    if (instructionCount >= instructions.size()) return;

    std::string logPath = "logs/" + screenInfo.getName() + ".txt";

    // Initialize detailed log file if first instruction
    if (instructionCount == 0) {
        std::ofstream log(logPath);
        log << "Process Name: " << screenInfo.getName() << "\n"
            << "Initial Assigned Core: " << coreID << "\n"
            << "Execution Log:\n\n";
        log.close();
    }

    // Execute instruction and log it
    auto instr = instructions[instructionCount];
    instr->execute(*this); // Pass the process object itself to the command

    // Write detailed log to file
    std::ofstream log(logPath, std::ios::app);
    log << "[" << ScreenInfo::getCurrentTimestamp() << "] "
        << "Process: " << screenInfo.getName()
        << " | Core: " << screenInfo.getCoreID()
        << " | Line: " << instructionCount + 1
        << " | Instruction: " << instr->toString()
        << "\n";
    log.close();

    instructionCount++;
    screenInfo.setCurrentLine(instructionCount);

    std::this_thread::sleep_for(std::chrono::milliseconds(delayPerInstruction));
}

bool Process::isComplete() const {
    return instructionCount >= instructions.size();
}

int Process::getID() const {
    return id;
}

ScreenInfo& Process::getScreenInfo(){
    return screenInfo;
}

void Process::updateScreenInfo() {
    screenInfo.setTimestamp(ScreenInfo::getCurrentTimestamp());
    screenInfo.setCoreID(coreID);
}

// --- New methods for 'process-smi' ---

// Adds a log message from a PRINT command to the in-memory buffer.
void Process::addLog(const std::string& log_message) {
    std::lock_guard<std::mutex> lock(log_mutex);
    print_logs.push_back(log_message);
}

// Safely retrieves all logs from the buffer.
std::vector<std::string> Process::getLogs() {
    std::lock_guard<std::mutex> lock(log_mutex);
    return print_logs;
}

// Gets the total number of instructions for the process.
int Process::getTotalInstructions() const {
    return instructions.size();
}

// Provides access to the process's variables map.
std::unordered_map<std::string, int32_t>& Process::getVariables() {
    return variables;
}

int Process::getCurrentPage() const {
    int page = instructionCount * pagesRequired / static_cast<int>(instructions.size());

    return std::min(page, pagesRequired - 1);
}
