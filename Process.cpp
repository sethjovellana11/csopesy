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

void Process::setIsAllocated(bool isAlloc) {
    memoryAllocated = isAlloc;
}

bool Process::getIsAllocated() {
    return memoryAllocated;
}

void Process::assignCore(int coreID) {
    this->coreID = coreID;
    screenInfo.setCoreID(coreID);
}

void Process::addInstruction(std::shared_ptr<ICommand> instr) {
    instructions.push_back(instr);
}

// Execute the next instruction
void Process::executeNextInstruction() {
    if (!std::filesystem::exists("logs")) {
        std::filesystem::create_directory("logs");
    }

    if (instructionCount >= instructions.size()) return;

    std::string logPath = "logs/" + screenInfo.getName() + ".txt";

    if (instructionCount == 0) {
        std::ofstream log(logPath);
        log << "Process Name: " << screenInfo.getName() << "\n"
            << "Initial Assigned Core: " << coreID << "\n"
            << "Execution Log:\n\n";
        log.close();
    }

    auto instr = instructions[instructionCount];
    instr->execute(*this);

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

ScreenInfo& Process::getScreenInfo() {
    return screenInfo;
}

void Process::updateScreenInfo() {
    screenInfo.setTimestamp(ScreenInfo::getCurrentTimestamp());
    screenInfo.setCoreID(coreID);
}

// --- screen-smi support methods ---
void Process::addLog(const std::string& log_message) {
    std::lock_guard<std::mutex> lock(log_mutex);
    print_logs.push_back(log_message);
}

std::vector<std::string> Process::getLogs() {
    std::lock_guard<std::mutex> lock(log_mutex);
    return print_logs;
}

int Process::getTotalInstructions() const {
    return instructions.size();
}

std::unordered_map<std::string, int32_t>& Process::getVariables() {
    return variables;
}

// --- Memory size support ---
void Process::setMemorySize(int size) {
    memorySizeBytes = size;
}

int Process::getMemorySize() const {
    return memorySizeBytes;
}
