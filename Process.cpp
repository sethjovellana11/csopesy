#include "Process.h"
#include <thread>
#include <chrono>
#include <fstream>

Process::Process(const std::string& name, int id)
    : id(id), instructionCount(0), screenInfo(name){
        screenInfo.setTotalLine(100);
    }

void Process::executeNextInstruction() {
    instructionCount++;
    screenInfo.setCurrentLine(instructionCount);

    std::ofstream log("logs/" + screenInfo.getName() + ".txt", std::ios::app);
    log << "[" << screenInfo.getCurrentTimestamp() << "] "
        << "CPU Running " << screenInfo.getName()
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
}
