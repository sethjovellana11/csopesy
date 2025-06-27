#include "Scheduler.h"
#include <iostream>
#include <algorithm>
#include <fstream> 

Scheduler::Scheduler(int coreCount, SchedulingMode mode, int quantum)
    : coreCount(coreCount), quantumCount(quantum), mode(mode), running(false), isCreatingProcesses(false) {}

Scheduler::Scheduler(int coreCount, SchedulingMode mode)
    : coreCount(coreCount), mode(mode), running(false), isCreatingProcesses(false) {}

Scheduler::~Scheduler() {}

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

void Scheduler::setDelay(int delay){
    delayPerInstruction = delay;
}

void Scheduler::createProcess(const std::string& procName, int instMin, int instMax){
    Process* p = new Process(procName, allProcesses.size() + 1);

    InstructionGenerator gen;
    int instCount = instMin + (rand() % (instMax - instMin + 1));
    auto instructions = gen.generateInstructions(instCount);

    for (auto& instr : instructions)
        p->addInstruction(instr);

    p->setDelay(delayPerInstruction);
    
    p->getScreenInfo().setTotalLine(instCount);
    this->addProcess(p);       
}

void Scheduler::createProcessesStart(int batch_process_freq, int instMin, int instMax) {
    if (isCreatingProcesses) return;

    isCreatingProcesses = true;
    
    processCreatorThread = std::thread([this, batch_process_freq, instMin, instMax] {
        int cycle = 0;
        int process_count = 0;

        while (isCreatingProcesses) {
            if (cycle == 0) {
                std::string name = "Process" + std::to_string(process_count + 1);
                this->createProcess(name, instMin, instMax);
                ++process_count;
            }

            ++cycle;
            cycle %= batch_process_freq;
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    });
}

void Scheduler::createProcessesStop() {
    isCreatingProcesses = false;

    if (processCreatorThread.joinable()) {
        processCreatorThread.join();
    }
}

void Scheduler::run() {
    running = true;
    for (int i = 0; i < coreCount; ++i) {
        cpuThreads.emplace_back(&Scheduler::cpuWorker, this, i);
    }
}

void Scheduler::stop() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        running = false;
    }

    cv.notify_all(); // wake up all waiting threads

    std::cout << "Stopping scheduler...\n";
    for (auto& t : cpuThreads)
        t.join();

    cpuThreads.clear(); // clean up for possible restart
}

void Scheduler::cpuWorker(int coreID) {
    while (true) {
        Process* current = nullptr;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] {
                return !processQueue.empty() || !running;
            });

            if (!running && processQueue.empty())
                return;

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
                std::lock_guard<std::mutex> lock(runningMutex);
                for (auto& s : runningScreens) {
                    if (s.getName() == current->getScreenInfo().getName()) {
                        s = current->getScreenInfo();
                        break;
                    }
                }
            }
        }

        if (mode == SchedulingMode::rr) {
            while (!current->isComplete() && running && processQueue.size() <= coreCount) 
            {
                current->executeNextInstruction();

                std::lock_guard<std::mutex> lock(runningMutex);
                for (auto& s : runningScreens) {
                    if (s.getName() == current->getScreenInfo().getName()) {
                        s = current->getScreenInfo();
                        break;
                    }
                }
            }
            for (int i = 0; i < quantumCount && !current->isComplete() && running; ++i) 
            {
                current->executeNextInstruction();
                
                std::lock_guard<std::mutex> lock(runningMutex);
                for (auto& s : runningScreens) {
                    if (s.getName() == current->getScreenInfo().getName()) {
                        s = current->getScreenInfo();  
                        break;
                    }
                }
            }
            if (!current->isComplete()) {
                addProcess(current); // requeue
            }
        }

        {
            current->assignCore(-1);
            std::lock_guard<std::mutex> lock(runningMutex);
            auto it = std::remove_if(runningScreens.begin(), runningScreens.end(),
                [&](const ScreenInfo& s) {
                    return s.getName() == current->getScreenInfo().getName();
                });
            runningScreens.erase(it, runningScreens.end());
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
                std::cout << "\n=== Running Processes " << screenName << " ===\n";
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


    //std::cout << "Screen \"" << screenName << "\" not found in running or finished screens.\n";
}

void Scheduler::printScreenList() const {
    int usedCores = 0;
    
    {
        std::lock_guard<std::mutex> lock(runningMutex);
        for (auto& screen : runningScreens) {
            if (screen.getCoreID() != -1) {
                usedCores++;
            }
        }
    }

    int availableCores = coreCount - usedCores;
    float utilization = coreCount > 0 ? (static_cast<float>(usedCores) / coreCount) * 100.0f : 0.0f;

    std::cout << "\n=== System Statistics ===\n";
    std::cout << "Total CPU Cores      : " << coreCount << "\n";
    std::cout << "Cores In Use         : " << usedCores << "\n";
    std::cout << "Available Cores      : " << availableCores << "\n";
    std::cout << "CPU Utilization      : " << utilization << "%\n";

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
    int usedCores = 0;
    
    {
        std::lock_guard<std::mutex> lock(runningMutex);
        for (auto& screen : runningScreens) {
            if (screen.getCoreID() != -1) {
                usedCores++;
            }
        }
    }

    int availableCores = coreCount - usedCores;
    float utilization = coreCount > 0 ? (static_cast<float>(usedCores) / coreCount) * 100.0f : 0.0f;

    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing.\n";
        return;
    }

    outFile << "\n=== System Statistics ===\n";
    outFile << "Total CPU Cores      : " << coreCount << "\n";
    outFile << "Cores In Use         : " << usedCores << "\n";
    outFile << "Available Cores      : " << availableCores << "\n";
    outFile << "CPU Utilization      : " << utilization << "%\n";

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