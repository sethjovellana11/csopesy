#pragma once
#include "Process.h"
#include "InstructionGenerator.h"
#include <queue>
#include <mutex>
#include <vector>
#include <thread>
#include <condition_variable>
#include <memory>

enum class SchedulingMode {
    fcfs,
    rr
};

class Scheduler {
public:
    Scheduler(int coreCount = 2, SchedulingMode mode = SchedulingMode::fcfs, int quantum = 5);
    ~Scheduler();

    void launch();
    void shutdown();

    void addProcess(Process* process);
    void printScreen(const std::string& screenName) const;
    void run();
    void stop();
    void printScreenList() const;

private:
    void cpuWorker(int coreID);

    std::queue<Process*> processQueue;
    std::vector<std::thread> cpuThreads;
    std::vector<ScreenInfo> finishedScreens;
    std::vector<ScreenInfo> runningScreens;

    mutable std::mutex queueMutex;
    mutable std::mutex finishedMutex;
    mutable std::mutex runningMutex;

    int quantumCount;
    int coreCount;
    SchedulingMode mode;
    bool running;
    std::condition_variable cv;

};