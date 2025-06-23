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
    Scheduler(int coreCount, SchedulingMode mode = SchedulingMode::rr, int quantum = 5);
    Scheduler(int coreCount, SchedulingMode mode = SchedulingMode::fcfs);
    ~Scheduler();

    void launch();
    void shutdown();

    void addProcess(Process* process);
    
    Process* findProcess(const std::string& name);
    void createProcessesStart(int batch_process_freq);
    void createProcessesStop();
    void writeScreenListToFile(const std::string& filename) const;
    void printScreen(const std::string& screenName) const;
    void run();
    void stop();
    void printScreenList() const;

private:
    void cpuWorker(int coreID);

    std::queue<Process*> processQueue;
    std::unordered_map<std::string, Process*> allProcesses;
    std::vector<std::thread> cpuThreads;
    std::vector<ScreenInfo> finishedScreens;
    std::vector<ScreenInfo> runningScreens;

    mutable std::mutex queueMutex;
    mutable std::mutex finishedMutex;
    mutable std::mutex runningMutex;
    mutable std::mutex allProcessMutex;

    int quantumCount;
    int coreCount;
    SchedulingMode mode;
    bool running;
    std::condition_variable cv;

    std::thread processCreatorThread;
    bool isCreatingProcesses;
};