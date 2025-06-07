#pragma once

#include <string>
#include <map>
#include "ScreenInfo.h"

class Emulator {
private:
    std::string input;
    std::map<std::string, ScreenInfo> screens;
    bool inScreen, inMarquee;
    std::string currentScreen;
    
    void clearScreen();
    void printHeader();
    void drawScreen(const std::string& name);
    void handleMainCommand(const std::string& input);
    void handleScreenCommand(const std::string& input);
    void handleMarqueeCommand(const std::string& input);

public:
    Emulator();
    void setInput(bool text);
    void run();
};
