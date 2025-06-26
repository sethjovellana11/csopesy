#include "Process.h"
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

Process::Process(const std::string& name, int id)
    : id(id), instructionCount(0), coreID(-1), screenInfo(name) {
}

void Process::assignCore(int coreID) {
    this->coreID = coreID;
    screenInfo.setCoreID(coreID);
}
void Process::addInstruction(std::shared_ptr<ICommand> instr) {
    instructions.push_back(instr);
}

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
    // Pass the process object itself to the command's execute method.
    instr->execute(*this);

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

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
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

// --- New methods for 'process-smi' & 'screen -r' ---

// Adds a log message from a PRINT command to the in-memory log buffer.
void Process::addLog(const std::string& log_message) {
    std::lock_guard<std::mutex> lock(log_mutex);
    print_logs.push_back(log_message);
}

// Safely retrieves all logs from the buffer for display.
std::vector<std::string> Process::getLogs() {
    std::lock_guard<std::mutex> lock(log_mutex);
    return print_logs;
}

// Returns the total number of instructions this process has.
int Process::getTotalInstructions() const {
    return instructions.size();
}

// Provides command access to the process's local variables.
std::unordered_map<std::string, int32_t>& Process::getVariables() {
    return variables;
}
