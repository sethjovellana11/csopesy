#include "Process.h"
#include <filesystem>
#include <fstream>
#include <thread>
#include <iostream>
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

std::string Process::getCurrentInstructionName() const {
    if (instructionCount >= instructions.size()) return "";
    return instructions[instructionCount]->getName();
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

void Process::incrementCurrentPage() {
    currentPage = (currentPage + 1) % pagesRequired;
}

uint16_t Process::readMemory(uint16_t address) const {
    if (address + 1 >= memorySize || (address % 2 != 0)) {
        throw std::runtime_error("Memory Access Violation: Invalid read at address 0x" + 
                                 std::to_string(address));
    }

    auto it = emulatedMemory.find(address);
    if (it != emulatedMemory.end()) {
        return it->second;
    }

    return 0; // uninitialized memory defaults to 0
}

void Process::writeMemory(uint16_t address, uint16_t value) {
    if (address + 1 >= memorySize || (address % 2 != 0)) {
        throw std::runtime_error("Memory Access Violation: Invalid write at address 0x" + 
                                 std::to_string(address));
    }

    emulatedMemory[address] = value;
}

bool Process::isTerminated() const {
    return terminated;
}

void Process::shutdown(const std::string& reason) {
    terminated = true;

    std::string logPath = "logs/" + screenInfo.getName() + ".txt";
    std::ofstream log(logPath, std::ios::app);

    log << "[!!! SHUTDOWN !!!] "
        << "Process: " << screenInfo.getName()
        << " | Reason: " << reason << "\n";
    log.close();

    addLog("[SHUTDOWN] Process terminated due to: " + reason);
    std::string timeStamp = screenInfo.getCurrentTimestamp();
    shutdownString = timeStamp + "[SHUTDOWN] Process terminated due to: " + reason;
}

void Process::displayShutdown(){
    std::cout << shutdownString << std::endl;
}
