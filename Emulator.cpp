#include "Emulator.h"
#include "Marquee.h"
#include "Scheduler.h"
#include <iostream>
#include <cstdlib>

Marquee* marquee = nullptr;
Scheduler* scheduler = nullptr;

Emulator::Emulator() : inScreen(false), currentScreen(""), inMarquee(false){}

void Emulator::clearScreen() {
    system("cls");
    if (!inScreen || inMarquee) {
        printHeader();
    }
}

void Emulator::setInput(bool text){
    inMarquee = text;
}

void Emulator::printHeader() {
    //        ___           ___           ___                         ___           ___                   
    //       /  /\         /  /\         /  /\          ___          /  /\         /  /\          __      
    //      /  /::\       /  /::\       /  /::\        /  /\        /  /::\       /  /::\        |  |\    
    //     /  /:/\:\     /__/:/\:\     /  /:/\:\      /  /::\      /  /:/\:\     /__/:/\:\       |  |:|   
    //    /  /:/  \:\   _\_ \:\ \:\   /  /:/  \:\    /  /:/\:\    /  /::\ \:\   _\_ \:\ \:\      |  |:|   
    //   /__/:/ \  \:\ /__/\ \:\ \:\ /__/:/ \__\:\  /  /::\ \:\  /__/:/\:\ \:\ /__/\ \:\ \:\     |__|:|__ 
    //   \  \:\  \__\/ \  \:\ \:\_\/ \  \:\ /  /:/ /__/:/\:\_\:\ \  \:\ \:\_\/ \  \:\ \:\_\/     /  /::::\
    //    \  \:\        \  \:\_\:\    \  \:\  /:/  \__\/  \:\/:/  \  \:\ \:\    \  \:\_\:\      /  /:/~~~~
    //     \  \:\        \  \:\/:/     \  \:\/:/        \  \::/    \  \:\_\/     \  \:\/:/     /__/:/     
    //      \  \:\        \  \::/       \  \::/          \__\/      \  \:\        \  \::/      \__\/      
    //       \__\/         \__\/         \__\/                       \__\/         \__\/     

    // Sourced from: https://patorjk.com/software/taag/, font: Isometric4

        std::cout << "\033[32m"; // making ascii green
        std::cout << "        ___           ___           ___                         ___           ___                   \n";
        std::cout << "       /  /\\         /  /\\         /  /\\          ___          /  /\\         /  /\\          __      \n";
        std::cout << "      /  /::\\       /  /::\\       /  /::\\        /  /\\        /  /::\\       /  /::\\        |  |\\    \n";
        std::cout << "     /  /:/\\:\\     /__/:/\\:\\     /  /:/\\:\\      /  /::\\      /  /:/\\:\\     /__/:/\\:\\       |  |:|   \n";
        std::cout << "    /  /:/  \\:\\   _\\_ \\:\\ \\:\\   /  /:/  \\:\\    /  /:/\\:\\    /  /::\\ \\:\\   _\\_ \\:\\ \\:\\      |  |:|   \n";
        std::cout << "   /__/:/ \\  \\:\\ /__/\\ \\:\\:\\:\\ /__/:/ \\__\\:\\  /  /::\\ \\:\\  /__/:/\\:\\ \\:\\ /__/\\ \\:\\:\\:\\     |__|:|__ \n";
        std::cout << "   \\  \\:\\  \\__\\/ \\  \\:\\ \\:\\_\\/ \\  \\:\\ /  /:/ /__/:/\\:\\_\\:\\ \\  \\:\\ \\:\\_\\/ \\  \\:\\ \\:\\_\\/     /  /::::\\\n";
        std::cout << "    \\  \\:\\        \\  \\:\\_\\:\\    \\  \\:\\  /:/  \\__\\\/  \\:\\/:/  \\  \\:\\ \\:\\    \\  \\:\\_\\:\\      /  /:/~~~~\n";
        std::cout << "     \\  \\:\\        \\  \\:\\/:/     \\  \\:\\/:/        \\  \\::/    \\  \\:\\_\\/     \\  \\:\\/:/     /__/:/     \n";
        std::cout << "      \\  \\:\\        \\  \\::/       \\  \\::/          \\__\\/      \\  \\:\\        \\  \\::/      \\__\\/      \n";
        std::cout << "       \\__\\/         \\__\\/         \\__\\/                       \\__\\/         \\__\\/                   \n";

        // change x in \033[xm to change color idk what looks nice haha (30-37 for foreground, 40-47 for background)
        std::cout << "\033[33mHello, welcome to CSOPESY commandline!\033[0m\n";
        std::cout << "\033[36mType 'exit' to quit, 'clear' to clear the screen.\033[0m\n";
}

void Emulator::listScreens() const {
    if (screens.empty()) {
        std::cout << "No screens available.\n";
    } else {
        std::cout << "=== Active Screen Sessions ===\n";
        for (const auto& pair : screens) {
            std::cout << "- " << pair.first << " (Line " << pair.second.getCurrentLine() 
                      << "/" << pair.second.getTotalLine() << ", Created: " 
                      << pair.second.getTimestamp() << ")\n";
        }
    }
}

void Emulator::drawScreen(const std::string& name) {
    clearScreen();
    inScreen = true;
    currentScreen = name;

    if (screens.find(name) == screens.end()) {
        screens[name] = ScreenInfo(name);
    }

    screens[name].display();
}

void Emulator::handleScreenCommand(const std::string& input) {
    if (input == "exit") {
        inScreen = false;
        currentScreen.clear();
        clearScreen();
    } else {
        std::cout << "Unknown screen command. Type 'exit' to return to main menu." << std::endl;
    }
}

/*void Emulator::handleMarqueeCommand(const std::string& input) {
    if (input == "exit") {
        inMarquee = false;
        marquee->stop();
        delete marquee;
        clearScreen();
        
    } else if (input.rfind("marquee-fps ", 0) == 0) {
        std::string val = input.substr(12);
        try {
            int newFPS = std::stoi(val);
            if (newFPS > 0 && marquee) {
                marquee->setFPS(newFPS);
                std::cout << "Updated marquee FPS to " << newFPS << "\n";
            } else {
                std::cout << "Invalid FPS value.\n";
            }
        } catch (...) {
            std::cout << "Invalid FPS value.\n";
        }
    } else {
        std::cout << "Unknown command. Try 'exit' or 'marquee-fps <value>'.\n";
    }
}*/

void Emulator::handleMainCommand(const std::string& input) {
    if (input == "initialize") {
        std::cout << "initialize command recognized. Doing something." << std::endl;
    }  
    else if (input == "marquee") {
        clearScreen();
        inMarquee = true;
    }
    else if (input == "screen") {
         std::cout << "screen command recognized. Doing something." << std::endl;
    }
    else if (input.rfind("screen -s ", 0) == 0) {
        std::string name = input.substr(10);
        if (!name.empty()) 
            drawScreen(name);
        else 
            std::cout << "Missing process name after 'screen -s'" << std::endl;
    } 
    else if (input.rfind("screen -r ", 0) == 0) {
        std::string name = input.substr(10);
        if(!scheduler){
            std::cout << "Scheduler not running yet" << std::endl;
        }
        else{
            clearScreen();
            scheduler->printScreen(name);
            inScreen = true;
        }
    } 
    else if (input == "screen -ls") {
        scheduler->printScreenList();
    }
    else if (input == "scheduler-test") {
        if (!scheduler) {
            scheduler = new Scheduler(4); 
                for (int i = 0; i < 10; ++i) {
                    std::string name = "Process" + std::to_string(i + 1);
                    Process p(name, i + 1);
                    scheduler->addProcess(p);
                }
                std::cout << "Starting scheduler with 10 processes...\n";
                std::thread([this]() { scheduler->run(); }).detach();
        } 
        else {
            std::cout << "Scheduler already running.\n";
        }
        //std::cout << "scheduler-test command recognized. Doing something." << std::endl;
    } 
    else if (input == "scheduler-stop") {
        std::cout << "scheduler-stop command recognized. Doing something." << std::endl;
    } 
    else if (input == "report-util") {
        std::cout << "report-util command recognized. Doing something." << std::endl;
    } 
    else if (input == "clear") {
        clearScreen();
    } 
    else if (input == "exit") {
        std::cout << "exit command recognized. Exiting program." << std::endl;
        exit(0);
    } 
    else if (input.empty()) {
         // Do nothing
    } else {
        std::cout << "Unknown command: " << input << std::endl;
    }
}


void Emulator::run() {
    std::string input;
    clearScreen();

    for (int i = 0; i < 10; ++i) {
        std::string processName = "Process_" + std::to_string(i + 1);
        processes.emplace_back(processName, i + 1);
    }

    while (true) {
        if (inScreen) {
            std::cout << "> ";
            std::getline(std::cin, input);
            handleScreenCommand(input);
        }
        else if(inMarquee){
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
        } 
        else {
            std::cout << "> ";
            std::getline(std::cin, input);
            handleMainCommand(input);
        }
    }
}