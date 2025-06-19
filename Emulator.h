#pragma once

#include <string>
#include <map>
#include <vector>
#include "Process.h"
#include "ScreenInfo.h"
#include <fstream>

class Emulator {
private:
    std::vector <Process> processes;
    std::string input;
    std::map<std::string, ScreenInfo> screens;
    bool inScreen, inMarquee;
    std::string currentScreen;
    bool isInitialized;

    // config variables
    int num_cpu, quantum_cycles, batch_process_freq, min_ins, max_ins, delay_per_exec;
    std::string scheduler_type;
    
    void clearScreen();
    void printHeader();
    void initialize();
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
