#include "Emulator.h"
#include "Marquee.h"
#include "Scheduler.h"
#include "InstructionGenerator.h"
#include <string>
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// External objects
Marquee* marquee = nullptr;
Scheduler* scheduler = nullptr;

Emulator::Emulator() : 
    inScreen(false), 
    currentScreen(""), 
    inMarquee(false), 
    isInitialized(false),
    batch_process_freq(0),
    delay_per_exec(0),
    max_ins(0),
    min_ins(0),
    num_cpu(0),
    quantum_cycles(0),
    scheduler(nullptr){}

void Emulator::clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        std::cout << "\033[2J\033[1;1H";
    #endif
    if (!inScreen || inMarquee) {
        printHeader();
    }
}

void Emulator::setInput(bool text){
    inMarquee = text;
}

void Emulator::printHeader() {
    std::cout << "\033[32m";
    std::cout << "        ___           ___           ___                         ___           ___                   \n";
    std::cout << "       /  /\\         /  /\\         /  /\\          ___          /  /\\         /  /\\          __      \n";
    std::cout << "      /  /::\\       /  /::\\       /  /::\\        /  /\\        /  /::\\       /  /::\\        |  |\\    \n";
    std::cout << "     /  /:/\\:\\     /__/:/\\:\\     /  /:/\\:\\      /  /::\\      /  /:/\\:\\     /__/:/\\:\\       |  |:|   \n";
    std::cout << "    /  /:/  \\:\\   _\\_ \\:\\ \\:\\   /  /:/  \\:\\    /  /:/\\:\\    /  /::\\ \\:\\   _\\_ \\:\\ \\:\\      |  |:|   \n";
    std::cout << "   /__/:/ \\  \\:\\ /__/\\ \\:\\:\\:\\ /__/:/ \\__\\:\\  /  /::\\ \\:\\  /__/:/\\:\\ \\:\\ /__/\\ \\:\\:\\:\\     |__|:|__ \n";
    std::cout << "   \\  \\:\\  \\__\\/ \\  \\:\\ \\:\\_\\/ \\  \\:\\ /  /:/ /__/:/\\:\\_\\:\\ \\  \\:\\ \\:\\_\\/ \\  \\:\\ \\:\\_\\/     /  /::::\\\n";
    std::cout << "    \\  \\:\\        \\  \\:\\_\\:\\    \\  \\:\\  /:/  \\__\\/  \\:\\/:/  \\  \\:\\ \\:\\    \\  \\:\\_\\:\\      /  /:/~~~~\n";
    std::cout << "     \\  \\:\\        \\  \\:\\/:/     \\  \\:\\/:/        \\  \\::/    \\  \\:\\_\\/     \\  \\:\\/:/     /__/:/     \n";
    std::cout << "      \\  \\:\\        \\  \\::/       \\  \\::/          \\__\\/      \\  \\:\\        \\  \\::/      \\__\\/      \n";
    std::cout << "       \\__\\/         \\__\\/         \\__\\/                       \\__\\/         \\__\\/                   \n";
    std::cout << "\033[33mHello, welcome to CSOPESY commandline!\033[0m\n";
    std::cout << "\033[36mCommands: 'initialize', 'scheduler-start', 'scheduler-stop', 'screen', 'report-util', 'clear', 'exit'.\033[0m\n";
}

void Emulator::initialize() {
    std::ifstream config("config.txt");
    std::string line, key, value;
    
    while(config >> key) {
        config >> value;
        if (key == "num-cpu") this->num_cpu = std::stoi(value);
        else if (key == "scheduler") this->scheduler_type = value;
        else if (key == "quantum-cycles") this->quantum_cycles = std::stoi(value);
        else if (key == "batch-process-freq") this->batch_process_freq = std::stoi(value);
        else if (key == "min-ins") this->min_ins = std::stoi(value);
        else if (key == "max-ins") this->max_ins = std::stoi(value);
        else if (key == "delays-per-exec") this->delay_per_exec = std::stoi(value);
        else if (key == "max-overall-mem") this->max_overall_mem = std::stoi(value);
        else if (key == "mem-per-frame") this->mem_per_frame = std::stoi(value);
        else if (key == "mem-per-proc") this->mem_per_proc = std::stoi(value);
    }

    if(scheduler_type == "rr"){
        scheduler = new Scheduler(num_cpu, SchedulingMode::rr, quantum_cycles);
        std::cout << "ROUND ROBIN MODE" << std::endl;
    } else {
        scheduler = new Scheduler(num_cpu, SchedulingMode::fcfs, 0);
        std::cout << "FCFS MODE" << std::endl;
    }
    
    isInitialized = true;
    scheduler->setDelay(delay_per_exec);
    scheduler->init_mem_manager(this->max_overall_mem, this->mem_per_frame, this->mem_per_proc);
    std::cout << "Emulator Initialized!" << std::endl;
    config.close();
    std::thread([this]() { scheduler->run(); }).detach();
}

bool Emulator::checkInitialized() const {
    if (!this->isInitialized)
        std::cout << "Emulator not initialized!" << std::endl;
    return isInitialized;
}

void Emulator::showProcessSMI(const std::string& screenName) {
    if (!scheduler) {
        std::cout << "Scheduler not running." << std::endl;
        return;
    }

    Process* process = scheduler->findProcess(screenName);
    if (!process) {
        std::cout << "Could not find process: " << screenName << ". It might have finished." << std::endl;
        inScreen = false;
        currentScreen.clear();
        clearScreen();
        return;
    }

    clearScreen();

    std::cout << "Process name: " << process->getScreenInfo().getName() << std::endl;
    std::cout << "ID: " << process->getID() << std::endl;
    std::cout << "Logs:" << std::endl;

    std::vector<std::string> logs = process->getLogs();
    if (logs.empty()) {
        std::cout << " (No logs yet)" << std::endl;
    } else {
        for (const auto& log : logs) {
            std::cout << log << std::endl;
        }
    }

    std::cout << std::endl;

    if (process->isComplete()) {
         std::cout << "Finished!" << std::endl;
    } else {
        std::cout << "Current instruction line: " << process->getScreenInfo().getCurrentLine() << std::endl;
        std::cout << "Lines of code: " << process->getTotalInstructions() << std::endl;
    }
}

// New function to handle 'screen -r'
void Emulator::printProcessInfo(const std::string& screenName) {
    if (!scheduler) {
        std::cout << "Scheduler not running." << std::endl;
        return;
    }

    Process* process = scheduler->findProcess(screenName);

    // Check if the process exists and has not finished execution.
    if (!process || process->isComplete()) {
        std::cout << "Process " << screenName << " not found." << std::endl;
        return;
    }

    // Print the process information directly to the console.
    std::cout << "\n--- Status for " << screenName << " ---\n";
    std::cout << "Process name: " << process->getScreenInfo().getName() << std::endl;
    std::cout << "ID: " << process->getID() << std::endl;
    std::cout << "Logs:" << std::endl;

    std::vector<std::string> logs = process->getLogs();
    if (logs.empty()) {
        std::cout << " (No logs yet)" << std::endl;
    } else {
        for (const auto& log : logs) {
            std::cout << log << std::endl;
        }
    }

    std::cout << std::endl;
    
    std::cout << "Current instruction line: " << process->getScreenInfo().getCurrentLine() << std::endl;
    std::cout << "Lines of code: " << process->getTotalInstructions() << std::endl;
    std::cout << "--------------------------\n";
}

void Emulator::handleScreenCommand(const std::string& input) {
    if (input == "exit") {
        inScreen = false;
        currentScreen.clear();
        clearScreen();
    } else if (input == "process-smi") {
        if (!currentScreen.empty()) {
            showProcessSMI(currentScreen);
        }
    } else {
        std::cout << "Unknown screen command. Type 'exit' or 'process-smi'." << std::endl;
    }
}

void Emulator::handleMainCommand(const std::string& input) {
    if (input == "initialize") {
        initialize();
    } else if (input.rfind("screen -s ", 0) == 0) {
        if (checkInitialized()) {
            clearScreen();
            std::string name = input.substr(10);
            if (!name.empty()) {
                Process* p = scheduler->findProcess(name);
                if (p == nullptr) {
                    scheduler->createProcess(name, min_ins, max_ins);
                    p = scheduler->findProcess(name); 
                }
                
                if (p && p->isComplete()){
                     std::cout << "Process '" << name << "' has already finished. Cannot attach to screen." << std::endl;
                } else if (p) {
                    currentScreen = name;
                    inScreen = true;
                    p->getScreenInfo().display();
                    std::cout << std::endl;
                    //showProcessSMI(name);
                }
            } else {
                std::cout << "Missing process name after 'screen -s'" << std::endl;
            }
        } 
    } else if (input.rfind("screen -r ", 0) == 0) { // New command logic
        if (checkInitialized()) {
            clearScreen();
            std::string name = input.substr(10);
            if (!name.empty()) {
                Process* p = scheduler->findProcess(name);
                if (p && p->isComplete()){
                     std::cout << "Process '" << name << "' has already finished. Cannot attach to screen." << std::endl;
                } 
                else if(p !=  nullptr){
                    currentScreen = name;
                    inScreen = true;
                    p->getScreenInfo().display();
                    std::cout << std::endl;
                }
                else if (p == nullptr){
                     std::cout << "Process " << name << " does not exist, please use screen -s <process name> to create a new process." << std::endl;
                }
            } else {
                std::cout << "Missing process name after 'screen -r" << std::endl;
            }
        }
    } else if (input == "screen -ls") {
        if (checkInitialized()) {
            clearScreen();
            scheduler->printScreenList();
        } 
    } else if (input == "scheduler-start") {
        if (checkInitialized()) {
            if(!scheduler->getIsCreatingProcesses()){
                scheduler->createProcessesStart(batch_process_freq, min_ins, max_ins);
                std::cout << "Started Processing" << std::endl;
            }
            else{
                std::cout << "Scheduler is already running, please use 'scheduler-stop' to stop the scheduler first" << std::endl;
            }
        }
    } else if (input == "scheduler-stop") {
        if (checkInitialized()){
            scheduler->createProcessesStop();
            std::cout << "Stopped Processing" << std::endl;
        }
    } else if (input == "report-util") {
        if (checkInitialized())
            scheduler->writeScreenListToFile("csopesylog.txt");
    } else if (input == "clear") {
        clearScreen();
    } else if (input == "exit") {
        std::cout << "Exiting emulator..." << std::endl;
        shouldExit = true;
    } else if (input.empty()) {
         // Do nothing
    } else {
        std::cout << "Unknown command: " << input << std::endl;
    }
}

void Emulator::run() {
    std::string input;
    clearScreen();
    while (!shouldExit) {
        if (inScreen) {
            std::cout << "root:\\> ";
            std::getline(std::cin, input);
            handleScreenCommand(input);
        } else if (inMarquee) {
            if (!marquee) {
                marquee = new Marquee("Hello World", 60, 10);
                marquee->start();
            } else if (!marquee->isRunning()) {
                marquee->stop();
                delete marquee;
                marquee = nullptr;
                inMarquee = false;
                clearScreen(); 
            }
        } else {
            std::cout << "> ";
            std::getline(std::cin, input);
            handleMainCommand(input);
        }
    }

    if (scheduler) {
        scheduler->createProcessesStop();
        scheduler->stop();
        delete scheduler;
        scheduler = nullptr;
    }

    std::cout << "Scheduler Destroyed" << std::endl;

    if (marquee) {
        marquee->stop();
        delete marquee;
        marquee = nullptr;
    }

    std::cout << "Goodbye.\n";
}
