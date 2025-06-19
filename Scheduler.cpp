#include "Scheduler.h"
#include <iostream>
#include <algorithm>

Scheduler::Scheduler(int coreCount) : coreCount(coreCount), running(true), isCreatingProcesses(false){}

void Scheduler::addProcess(const Process& process) {
    std::lock_guard<std::mutex> lock(queueMutex);
    processQueue.push(process);
}

void Scheduler::createProcessesStart(int batch_process_freq) {
    isCreatingProcesses = true;
    int cycle = 0;
    int process_count = 0;

    while (isCreatingProcesses) {
        if (cycle == 0) {
            // create processes using addProcess
        }
        
        cycle++;
        cycle %= batch_process_freq;
    }
}

void Scheduler::createProcessesStop() {
    isCreatingProcesses = false;
}

void Scheduler::run() {
    for (int i = 0; i < coreCount; ++i)
        cpuThreads.emplace_back(&Scheduler::cpuWorker, this, i);

    for (auto& t : cpuThreads)
        t.join();
}

void Scheduler::cpuWorker(int coreID) {
    while (true) {
        Process current("", -1);
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (processQueue.empty()) break;
            current = processQueue.front();
            current.assignCore(coreID);
            runningScreens.push_back(current.getScreenInfo());
            processQueue.pop();
        }

        while (!current.isComplete()) {
            current.executeNextInstruction();

            std::lock_guard<std::mutex> lock(runningMutex);
            for (auto& s : runningScreens) {
                if (s.getName() == current.getScreenInfo().getName()) {
                    s = current.getScreenInfo();  
                    break;
                }
            }
        }

        {
            current.assignCore(-1); //unassign core
            std::lock_guard<std::mutex> lock(runningMutex);
            auto it = std::remove_if(runningScreens.begin(), runningScreens.end(),
                [&](const ScreenInfo& s) { return s.getName() == current.getScreenInfo().getName(); });
            runningScreens.erase(it, runningScreens.end());
        }

        {
            std::lock_guard<std::mutex> lock(finishedMutex);
            finishedScreens.push_back(current.getScreenInfo());
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
                std::cout << "\n=== Finished Screen: " << screenName << " ===\n";
                screen.display();
                return;
            }
        }
    }

    std::cout << "Screen \"" << screenName << "\" not found in running or finished screens.\n";
}

void Scheduler::printScreenList() const {
    std::cout << "\n=== Running Screens ===\n";
    {
        std::lock_guard<std::mutex> lock(runningMutex);
        if (runningScreens.empty()) {
            std::cout << "No screens are currently running.\n";
        } else {
            for (const auto& screen : runningScreens) {
                screen.display();
            }
        }
    }

    std::cout << "\n=== Finished Screens ===\n";
    {
        std::lock_guard<std::mutex> lock(finishedMutex);
        if (finishedScreens.empty()) {
            std::cout << "No finished screens.\n";
        } else {
            for (const auto& screen : finishedScreens) {
                screen.display();
            }
        }
    }
}