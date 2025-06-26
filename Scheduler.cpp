#include "Scheduler.h"
#include <iostream>
#include <algorithm>
#include <fstream> 

// The single constructor implementation.
Scheduler::Scheduler(int coreCount, SchedulingMode mode, int quantum)
    : coreCount(coreCount), quantumCount(quantum), mode(mode), running(false), isCreatingProcesses(false) {}

Scheduler::~Scheduler() {
    stop();
}

void Scheduler::run() {
    running = true;
    for (int i = 0; i < coreCount; ++i) {
        cpuThreads.emplace_back(&Scheduler::cpuWorker, this, i);
    }
}

void Scheduler::stop() {
    isCreatingProcesses = false;
    if (processCreatorThread.joinable()) {
        processCreatorThread.join();
    }
    
    running = false;
    cv.notify_all();

    for (auto& t : cpuThreads) {
        if(t.joinable()) t.join();
    }
}

void Scheduler::addProcess(Process* process) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        processQueue.push(process);
    }
    {
        std::lock_guard<std::mutex> lock(allProcessMutex);
        allProcesses[process->getScreenInfo().getName()] = process;
    }
    cv.notify_one();
}

Process* Scheduler::findProcess(const std::string& name) {
    std::lock_guard<std::mutex> lock(allProcessMutex);
    auto it = allProcesses.find(name);
    if (it != allProcesses.end()) {
        return it->second;
    }
    return nullptr;
}

void Scheduler::createProcess(const std::string& procName, int instMin, int instMax){
    // Use a lock to safely get the current size for the new process ID
    std::lock_guard<std::mutex> lock(allProcessMutex);
    Process* p = new Process(procName, allProcesses.size() + 1);

    InstructionGenerator gen;
    int instCount = instMin + (rand() % (instMax - instMin + 1));
    auto instructions = gen.generateInstructions(instCount);

    for (auto& instr : instructions)
        p->addInstruction(instr);
    
    p->getScreenInfo().setTotalLine(instCount);
    
    // The addProcess method is thread-safe, so no extra lock needed here.
    this->addProcess(p);       
}

void Scheduler::createProcessesStart(int batch_process_freq, int instMin, int instMax) {
    if (isCreatingProcesses) return;
    isCreatingProcesses = true;
    
    processCreatorThread = std::thread([this, batch_process_freq, instMin, instMax] {
        int process_count = 0;
        {
             // Safely get the initial process count
            std::lock_guard<std::mutex> lock(allProcessMutex);
            process_count = allProcesses.size();
        }

        while (isCreatingProcesses) {
            // Updated naming scheme to "Process<N>"
            std::string name = "Process" + std::to_string(process_count + 1);
            this->createProcess(name, instMin, instMax);
            ++process_count;
            
            // Wait for the specified frequency
            for (int i = 0; i < batch_process_freq && isCreatingProcesses; ++i) {
                 std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    });
}

void Scheduler::createProcessesStop() {
    isCreatingProcesses = false;
    if (processCreatorThread.joinable()) {
        processCreatorThread.join();
    }
}

void Scheduler::cpuWorker(int coreID) {
    while (running) {
        Process* current = nullptr;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] {
                return !processQueue.empty() || !running;
            });

            if (!running && processQueue.empty()) return;
            if (processQueue.empty()) continue;
            
            current = processQueue.front();
            processQueue.pop();
        }

        current->assignCore(coreID);
        
        {
            std::lock_guard<std::mutex> lock(runningMutex);
            runningScreens.push_back(current->getScreenInfo());
        }

        if (mode == SchedulingMode::fcfs) {
            while (!current->isComplete() && running) {
                current->executeNextInstruction();
            }
        } else if (mode == SchedulingMode::rr) {
            for (int i = 0; i < quantumCount && !current->isComplete() && running; ++i) {
                current->executeNextInstruction();
            }
            if (!current->isComplete()) {
                addProcess(current);
            }
        }
        
        {
            current->assignCore(-1); // Mark as no longer on a core
            std::lock_guard<std::mutex> lock(runningMutex);
            auto it = std::remove_if(runningScreens.begin(), runningScreens.end(),
                [&](const ScreenInfo& s) { return s.getName() == current->getScreenInfo().getName(); });
            if (it != runningScreens.end()) {
                runningScreens.erase(it, runningScreens.end());
            }
        }

        if (current->isComplete()) {
            std::lock_guard<std::mutex> lock(finishedMutex);
            finishedScreens.push_back(current->getScreenInfo());
        }
    }
}

void Scheduler::printScreen(const std::string& screenName) const {
    {
        std::lock_guard<std::mutex> lock(runningMutex);
        for (const auto& screen : runningScreens) {
            if (screen.getName() == screenName) {
                std::cout << "\n=== Running Process " << screenName << " ===\n";
                screen.display();
                return;
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(finishedMutex);
        for (const auto& screen : finishedScreens) {
            if (screen.getName() == screenName) {
                std::cout << "\n=== Finished Process: " << screenName << " ===\n";
                screen.display();
                return;
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        std::queue<Process*> tempQueue = processQueue;
        while (!tempQueue.empty()) {
            Process* p = tempQueue.front();
            tempQueue.pop();
            if (p->getScreenInfo().getName() == screenName) {
                std::cout << "\n=== Queued Process: " << screenName << " ===\n";
                p->getScreenInfo().display();
                return;
            }
        }
    }
}

void Scheduler::printScreenList() const {
    std::cout << "\n=== Queued Process ===\n";
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (processQueue.empty()) {
            std::cout << "No queued process.\n";
        } else {
            std::queue<Process*> tempQueue = processQueue;
            while (!tempQueue.empty()) {
                Process* p = tempQueue.front();
                tempQueue.pop();
                p->getScreenInfo().display();
            }
        }
    }

    std::cout << "\n=== Running Process ===\n";
    {
        std::lock_guard<std::mutex> lock(runningMutex);
        if (runningScreens.empty()) {
            std::cout << "No process are currently running.\n";
        } else {
            for (const auto& screen : runningScreens) {
                screen.display();
            }
        }
    }

    std::cout << "\n=== Finished Process ===\n";
    {
        std::lock_guard<std::mutex> lock(finishedMutex);
        if (finishedScreens.empty()) {
            std::cout << "No finished process.\n";
        } else {
            for (const auto& screen : finishedScreens) {
                screen.display();
            }
        }
    }
}

void Scheduler::writeScreenListToFile(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing.\n";
        return;
    }

    outFile << "=== Queued Process ===\n";
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (processQueue.empty()) {
            outFile << "No queued process.\n";
        } else {
            std::queue<Process*> tempQueue = processQueue;
            while (!tempQueue.empty()) {
                Process* p = tempQueue.front();
                tempQueue.pop();
                outFile << p->getScreenInfo().toString();
            }
        }
    }

    outFile << "\n=== Running Process ===\n";
    {
        std::lock_guard<std::mutex> lock(runningMutex);
        if (runningScreens.empty()) {
            outFile << "No processes are currently running.\n";
        } else {
            for (const auto& screen : runningScreens) {
                outFile << screen.toString();
            }
        }
    }

    outFile << "\n=== Finished Process ===\n";
    {
        std::lock_guard<std::mutex> lock(finishedMutex);
        if (finishedScreens.empty()) {
            outFile << "No finished process.\n";
        } else {
            for (const auto& screen : finishedScreens) {
                outFile << screen.toString();
            }
        }
    }

    outFile.close();
    std::cout << "Process report written to " << filename << std::endl;
}
