#ifndef EMULATOR_H
#define EMULATOR_H

#include <string>
#include <map>
#include "ScreenInfo.h"

class Emulator {
private:
    std::map<std::string, ScreenInfo> screens;
    bool inScreen;
    std::string currentScreen;

    void clearScreen();
    void printHeader();
    void drawScreen(const std::string& name);
    void handleMainCommand(const std::string& input);
    void handleScreenCommand(const std::string& input);

public:
    Emulator();
    void run();
};

#endif