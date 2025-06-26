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
    std::filesystem::create_directory("logs");

    if (instructionCount >= instructions.size()) return;

    std::string logPath = "logs/" + screenInfo.getName() + ".txt";

    // Initialize log file if first instruction
    if (instructionCount == 0) {
        std::ofstream log(logPath);
        log << "Process Name: " << screenInfo.getName() << "\n"
            << "Initial Assigned Core: " << coreID << "\n"
            << "Execution Log:\n\n";
        log.close();
    }

    // Execute instruction and log it
    auto instr = instructions[instructionCount];
    instr->execute(variables);

    std::ofstream log(logPath, std::ios::app);
    log << "[" << screenInfo.getCurrentTimestamp() << "] "
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
    return instructionCount >= instructions.size(); // actual count
}

int Process::getID() const {
    return id;
}

ScreenInfo& Process::getScreenInfo(){
    return screenInfo;
}

void Process::updateScreenInfo() {
    screenInfo.setTimestamp(screenInfo.getCurrentTimestamp());
    screenInfo.setCoreID(coreID);
}
