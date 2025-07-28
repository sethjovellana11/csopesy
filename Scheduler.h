#pragma once
#include "Process.h"
#include "MemoryManager.h"
#include "InstructionGenerator.h"
#include <queue>
#include <mutex>
#include <vector>
#include <thread>
#include <filesystem>
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

    void init_mem_manager(int max_overall_mem, int mem_per_frame, int min_mem_per_proc, int max_mem_per_proc);

    void addProcess(Process* process);
    void createProcess(const std::string& procName, int instMin, int instMax);
    
    Process* findProcess(const std::string& name);
    void createProcessesStart(int batch_process_freq, int instMin, int instMax);
    void createProcessesStop();
    void writeScreenListToFile(const std::string& filename) const;
    void printScreen(const std::string& screenName) const;
    void setDelay(int delay);
    void run();
    void stop();
    bool getIsCreatingProcesses() const;

    void printScreenList() const;
    void printProcessSmi() const;
    void printVMStats() const;

    void printMemoryStatus() const;

private:
    MemoryManager memManager = MemoryManager(16384, 16, 265, 1024); // DEFAULT VALUES
    int cycleCounter = 0;

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

    int process_count = 0;
    int quantumCount;
    int coreCount;
    int delayPerInstruction; 
    SchedulingMode mode;
    bool running;
    std::condition_variable cv;

    std::thread processCreatorThread;
    bool isCreatingProcesses = false;

    //ticks
    size_t activeTicks = 0;
    size_t idleTicks = 0;
};