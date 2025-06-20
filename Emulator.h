#pragma once

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include "InstructionGenerator.h"
#include "Process.h"
#include "ScreenInfo.h"

class Emulator {
private:
    std::vector <Process> processes;
    std::string input;
    std::map<std::string, ScreenInfo> screens;
    bool inScreen, inMarquee;
    std::string currentScreen;
    std::unordered_map<std::string, Process*> activeProcesses;
    
    void clearScreen();
    void printHeader();
    void drawScreen(const std::string& name);
    void handleMainCommand(const std::string& input);
    void handleScreenCommand(const std::string& input);
    void handleMarqueeCommand(const std::string& input);
    void listScreens() const;

public:
    Emulator();
    void setInput(bool text);
    void run();
};
