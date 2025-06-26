#pragma once
#include "Process.h"
#include "InstructionGenerator.h"
#include <queue>
#include <mutex>
#include <vector>
#include <thread>
#include <condition_variable>
#include <memory>
#include <atomic>

enum class SchedulingMode {
    fcfs,
    rr
};

class Scheduler {
public:
    // This is now the only constructor. It handles both RR and FCFS.
    // For FCFS, the 'quantum' value is ignored.
    Scheduler(int coreCount, SchedulingMode mode = SchedulingMode::rr, int quantum = 5);
    ~Scheduler();

    void addProcess(Process* process);
    void createProcess(const std::string& procName, int instMin, int instMax);
    
    Process* findProcess(const std::string& name);
    void createProcessesStart(int batch_process_freq, int instMin, int instMax);
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
    std::atomic<bool> running;
    std::atomic<bool> isCreatingProcesses;

    std::condition_variable cv;
    std::thread processCreatorThread;
};
