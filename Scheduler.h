#pragma once
#include "Process.h"
#include "Scheduler.h"
#include <queue>
#include <mutex>
#include <vector>
#include <thread>
#include <memory>

class Scheduler {
public:
    Scheduler(int coreCount);
    void addProcess(const Process& process);
    void createProcessesStart(int batch_process_freq);
    void createProcessesStop();
    void printScreen(const std::string& screenName) const;
    void run();
    void printScreenList() const;

private:
    void cpuWorker(int coreID);

    std::queue<Process> processQueue;
    std::vector<std::thread> cpuThreads;
    std::vector<ScreenInfo> finishedScreens;
    std::vector<ScreenInfo> runningScreens;

    std::mutex queueMutex;
    mutable std::mutex finishedMutex;
    mutable std::mutex runningMutex;

    int coreCount;
    bool running;
    bool isCreatingProcesses;
};