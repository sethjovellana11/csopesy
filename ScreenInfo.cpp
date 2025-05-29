#include "ScreenInfo.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

ScreenInfo::ScreenInfo()
    : name("Unnamed"), currentLine(1), totalLine(10), timestamp(getCurrentTimestamp()) {}

ScreenInfo::ScreenInfo(const std::string& name)
    : name(name), currentLine(1), totalLine(10), timestamp(getCurrentTimestamp()) {}

std::string ScreenInfo::getCurrentTimestamp() {
    time_t now = time(nullptr);
    tm* ltm = localtime(&now);
    std::stringstream ss;
    ss << std::put_time(ltm, "%m/%d/%Y, %I:%M:%S %p");
    return ss.str();
}
  
void ScreenInfo::display() const {
    std::cout << "=== SCREEN: " << name << " ===" << std::endl;
    std::cout << "Process Name       : " << name << std::endl;
    std::cout << "Instruction Line   : " << currentLine << " / " << totalLine << std::endl;
    std::cout << "Created At         : " << timestamp << std::endl;
    std::cout << "Type 'exit' to return to main menu.\n" << std::endl;
}

const std::string& ScreenInfo::getName() const {
    return name;
}