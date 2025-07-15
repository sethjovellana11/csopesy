#pragma once

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include "InstructionGenerator.h"
#include "Process.h"
#include "ScreenInfo.h"
#include <fstream>

// Forward declaration
class Scheduler;

class Emulator {
private:
    std::vector <Process> processes;
    std::string input;
    std::map<std::string, ScreenInfo> screens;
    bool inScreen, inMarquee;
    std::string currentScreen;
    std::unordered_map<std::string, Process*> activeProcesses;
    bool isInitialized;
    bool shouldExit = false;

    // config variables
    int num_cpu, quantum_cycles, batch_process_freq, min_ins, max_ins, delay_per_exec,
        max_overall_mem, mem_per_frame, mem_per_proc;
    std::string scheduler_type;
    
    // External objects
    Scheduler* scheduler;

    void clearScreen();
    void printHeader();
    void initialize();
    void handleMainCommand(const std::string& input);
    void handleScreenCommand(const std::string& input);
    void listScreens() const;
    bool checkInitialized() const;
    
    // Helpers for process viewing
    void showProcessSMI(const std::string& screenName);
    void printProcessInfo(const std::string& screenName); // New helper for screen -r

public:
    Emulator();
    void setInput(bool text);
    void run();
};
