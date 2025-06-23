#include "Scheduler.h"
#include <iostream>
#include <algorithm>

Scheduler::Scheduler(int coreCount, SchedulingMode mode, int quantum)
    : coreCount(coreCount), quantumCount(quantum), mode(mode), running(false), isCreatingProcesses(false) {}

Scheduler::~Scheduler() {}

void Scheduler::addProcess(Process* process) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        processQueue.push(process);
    }
    cv.notify_one();
}

void Scheduler::createProcessesStart(int batch_process_freq) {
    // create lambda function and call it as a thread
    std::thread processCreator([this, batch_process_freq] {
        this->isCreatingProcesses = true;
        int cycle = 0;
        int process_count = 0;

        while (isCreatingProcesses) {
            if (cycle == 0) {
                // create processes using addProcess
            }

            cycle++;
            cycle %= batch_process_freq;
        }
        });
    
    processCreator.join();
}

void Scheduler::createProcessesStop() {
    isCreatingProcesses = false;
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
                /*for (auto& s : runningScreens) {
                    if (s.getName() == current->getScreenInfo().getName()) {
                        s = current->getScreenInfo();
                        break;
                    }
                }*/
            }
        }

        if (mode == SchedulingMode::rr) {
            if(processQueue.size() > coreCount)
            {
                for (int i = 0; i < quantumCount && !current->isComplete() && running; ++i)
                {
                    current->executeNextInstruction();
                    //std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            else
            {
                current->executeNextInstruction();
                //std::this_thread::sleep_for(std::chrono::milliseconds(10));
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