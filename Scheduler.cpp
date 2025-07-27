#include "Scheduler.h"
#include "MemoryManager.h"
#include <iostream>
#include <algorithm>
#include <fstream> 
#include <filesystem>

Scheduler::Scheduler(int coreCount, SchedulingMode mode, int quantum)
    : coreCount(coreCount), quantumCount(quantum), mode(mode), running(false), isCreatingProcesses(false) {}

Scheduler::Scheduler(int coreCount, SchedulingMode mode)
    : coreCount(coreCount), mode(mode), running(false), isCreatingProcesses(false) {}

Scheduler::~Scheduler() {
    stop();
    cv.notify_all();
}

void Scheduler::init_mem_manager(int max_overall_mem, int mem_per_frame, int min_mem_per_proc, int max_mem_per_proc) {
    this->memManager = MemoryManager(max_overall_mem, mem_per_frame, min_mem_per_proc, max_mem_per_proc);
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

void Scheduler::setDelay(int delay){
    delayPerInstruction = delay;
}

bool Scheduler::getIsCreatingProcesses() const{
    return isCreatingProcesses;
}

void Scheduler::createProcess(const std::string& procName, int instMin, int instMax){
    Process* p = new Process(procName, allProcesses.size() + 1);

    InstructionGenerator gen;
    int instCount = instMin + (rand() % (instMax - instMin + 1));
    auto instructions = gen.generateInstructions(instCount);

    for (auto& instr : instructions)
        p->addInstruction(instr);

    p->setDelay(delayPerInstruction);

    int memSize = memManager.randomMemoryForProcess(); 
    int pages = memManager.calculatePagesRequired(memSize); 
    p->setPagesRequired(pages);
    p->setMemory(memSize);
    p->getScreenInfo().setTotalLine(instCount);
    p->getScreenInfo().setTotalMem(memSize);
    this->addProcess(p);       
}

void Scheduler::createProcessesStart(int batch_process_freq, int instMin, int instMax) {
    if (isCreatingProcesses) return;

    isCreatingProcesses = true;
    
    processCreatorThread = std::thread([this, batch_process_freq, instMin, instMax] {
        int cycle = 0;

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

    std::cout << "[Scheduler] Notifying all CPU workers to exit..." << std::endl;
    cv.notify_all();

    for (auto& t : cpuThreads) {
        std::cout << "[Scheduler] Joining CPU thread..." << std::endl;
        t.join();
    }

    std::cout << "[Scheduler] All CPU threads stopped." << std::endl;
    cpuThreads.clear();
}

void Scheduler::cpuWorker(int coreID) {
    int cycleCounter = 0;

    while (true) {
        Process* current = nullptr;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            while (processQueue.empty()) {
                idleTicks++;
                if (!running) return;
                cv.wait(lock);
            }

            current = processQueue.front();
            processQueue.pop();
        }

        // Used for regular paging but not for demand paging
        /*
        if (!current->getIsAllocated()) {
            if (!memManager.allocate(current->getID())) {
                addProcess(current);  // Retry later
                continue;
            }
            current->setIsAllocated(true);
        }
        */

        current->assignCore(coreID);
        {
            std::lock_guard<std::mutex> lock(runningMutex);
            runningScreens.push_back(current->getScreenInfo());
        }

        if (mode == SchedulingMode::fcfs) {
            while (!current->isComplete() && running) {
                int page = current->getCurrentPage();

                bool success = memManager.accessPage(current->getID(), page);
                if (!success) {
                    std::cout << "[Page Fault] Process " << current->getID() << " requesting page " << page << "\n";
                }

                current->executeNextInstruction();
                activeTicks++;

                {
                    std::lock_guard<std::mutex> lock(runningMutex);
                    for (auto& s : runningScreens) {
                        if (s.getName() == current->getScreenInfo().getName()) {
                            s = current->getScreenInfo();
                            break;
                        }
                    }
                }
            }
        }

        if (mode == SchedulingMode::rr) {
            for (int i = 0; i < quantumCount && !current->isComplete() && running; ++i) {
                int page = current->getCurrentPage();

                bool success = memManager.accessPage(current->getID(), page);

                if(!memManager.isProcessInRegistry(current->getID()))
                {
                    //std::cout << "Process: " << current->getID() << " with memory per page " << current->getMemPerPage() << " Added\n"; 
                    memManager.registerProcess(current);
                }

                if (!success) {
                    std::cout << "[Page Fault] Process " << current->getID() << " requesting page " << page << "\n";
                }

                current->executeNextInstruction();
                activeTicks++;
                {
                    std::lock_guard<std::mutex> lock(runningMutex);
                    for (auto& s : runningScreens) {
                        if (s.getName() == current->getScreenInfo().getName()) {
                            s = current->getScreenInfo();
                            break;
                        }
                    }
                }

                // Snapshot every quantum cycle
                /*
                if (++cycleCounter % quantumCount == 0) {
                    std::string dir = "Memory_Stamp";
                    std::filesystem::create_directory(dir);

                    std::string filePath = dir + "/memory_stamp_" + std::to_string(cycleCounter) + ".txt";
                    std::ofstream out(filePath);
                    out << "Timestamp: " << ScreenInfo::getCurrentTimestamp() << "\n";
                    out << "Quantum Cycle: " << cycleCounter << "\n";
                    out << "Processes in memory: " << memManager.processesInMemory() << "\n";
                    out << "External fragmentation: " << memManager.getExternalFragmentation() << " KB\n";
                    out << "Memory Map:\n" << memManager.asciiMemoryMap() << "\n";
                    out.close();
                }
                */
            }

            if (!current->isComplete() && running) {
                addProcess(current);
            }
        }

        current->assignCore(-1);
        {
            std::lock_guard<std::mutex> lock(runningMutex);
            auto it = std::remove_if(runningScreens.begin(), runningScreens.end(),
                [&](const ScreenInfo& s) {
                    return s.getName() == current->getScreenInfo().getName();
                });
            runningScreens.erase(it, runningScreens.end());
        }

        if (current->isComplete()) {
            memManager.deallocate(current->getID());
            memManager.unregisterProcess(current->getID());
            current->setIsAllocated(false);
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

void Scheduler::printProcessSmi() const{
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
    float memoryutil = memManager.getTotalMemory() > 0 ? (static_cast<float>(memManager.getUsedMemory()) / memManager.getTotalMemory()) * 100.0f : 0.0f;

    std::cout << "\n=== System Statistics ===\n";
    std::cout << "Total CPU Cores        : " << coreCount << "\n";
    std::cout << "Cores In Use           : " << usedCores << "\n";
    std::cout << "Available Cores        : " << availableCores << "\n";
    std::cout << "CPU Utilization        : " << utilization << "%\n";
    std::cout << "Memory Usage           : " << memManager.getUsedMemory() << " kb " << "/ " << memManager.getTotalMemory() << " kb\n";
    std::cout << "Memory Utilization     : " << memoryutil << "%\n";

    std::cout << "\n=== Running Process and Memory ===\n";
    {
        std::lock_guard<std::mutex> lock(runningMutex);
        if (runningScreens.empty()) {
            std::cout << "No process are currently running.\n";
        } else {
            for (const auto& screen : runningScreens) {
                screen.displaySmi();
            }
        }
    }

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

void Scheduler::printVMStats() const {
    std::cout << "\n=== VM STATISTICS ===\n";
    std::cout << "Total memory: " << memManager.getTotalMemory() << " bytes\n";
    std::cout << "Used memory: " << memManager.getUsedMemory() << " bytes\n";
    std::cout << "Free memory: " << memManager.getFreeMemory() << " bytes\n";

    std::cout << "Idle CPU ticks: " << idleTicks << "\n";
    std::cout << "Active CPU ticks: " << activeTicks << "\n";
    std::cout << "Total CPU ticks: " << idleTicks + activeTicks << "\n";

    std::cout << "Num paged in: " << memManager.getPageIns() << "\n";
    std::cout << "Num paged out: " << memManager.getPageOuts() << "\n";
    std::cout << "======================\n";
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