#include "Process.h"
#include <thread>
#include <chrono>
#include <fstream>

Process::Process(const std::string& name, int id)
    : id(id), instructionCount(0), screenInfo(name), coreID(-1){
        screenInfo.setTotalLine(100);
        std::ofstream log("logs/" + screenInfo.getName() + ".txt", std::ios::app);
        log << "Process name: " << name << "\nLogs:\n\n";
        log.close();
    }

void Process::assignCore(int coreID) {
    this->coreID = coreID;
    this->screenInfo.setCoreID(coreID);
}

void Process::executeNextInstruction() {
    instructionCount++;
    screenInfo.setCurrentLine(instructionCount);

    std::ofstream log("logs/" + screenInfo.getName() + ".txt", std::ios::app);
    log << "[" << screenInfo.getCurrentTimestamp() << "] "
        << "Core: " << screenInfo.getCoreID() << "\t"
        << " Instruction " << instructionCount << "\n";
    log.close();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

bool Process::isComplete() const {
    return instructionCount >= screenInfo.getTotalLine();
}

int Process::getID() const {
    return id;
}

const ScreenInfo& Process::getScreenInfo() const {
    return screenInfo;
}

void Process::updateScreenInfo() {
    screenInfo.setTimestamp(screenInfo.getCurrentTimestamp());
    screenInfo.setCoreID(coreID);
}
