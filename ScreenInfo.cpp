#include "ScreenInfo.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

ScreenInfo::ScreenInfo()
    : name("Unnamed"), currentLine(1), totalLine(100), timestamp(getCurrentTimestamp()) {}

ScreenInfo::ScreenInfo(const std::string& name)
    : name(name), currentLine(1), totalLine(100), timestamp(getCurrentTimestamp()) {}

std::string ScreenInfo::getCurrentTimestamp() {
    time_t now = time(nullptr);
    tm* ltm = localtime(&now);
    std::stringstream ss;
    ss << std::put_time(ltm, "%m/%d/%Y, %I:%M:%S %p");
    return ss.str();
}

void ScreenInfo::display() const {
    std::cout   <<  name << "\t" << "(" << timestamp << ")" << "\t"  
                << "Core: " << ""/*core id*/ << "\t"
                << currentLine << " / " << totalLine << std::endl;
}

const std::string& ScreenInfo::getName() const {
    return name;
}

int ScreenInfo::getCurrentLine() const {
    return currentLine;
}

int ScreenInfo::getTotalLine() const {
    return totalLine;
}

void ScreenInfo::setTotalLine(int line){
   totalLine = line;
}

void ScreenInfo::setCurrentLine(int line) {
    currentLine = line;
}

void ScreenInfo::setTimestamp(const std::string& ts) {
    timestamp = ts;
}

std::string ScreenInfo::getTimestamp() const {
    return timestamp;
}

void ScreenInfo::incrementLine() {
    if (currentLine < totalLine)
        currentLine++;
}