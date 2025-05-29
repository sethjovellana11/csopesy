#pragma once
#include <string>

class ScreenInfo{
private:
    std::string name;
    int currentLine;
    int totalLine;
    std:: string timestamp;
    std:: string getCurrentTimestamp();

public:
    ScreenInfo(); 
    ScreenInfo(const std::string& name);
    void display() const;
    const std::string& getName() const;
};

