#pragma once
#include <string>

class ScreenInfo {
public:
    ScreenInfo();
    explicit ScreenInfo(const std::string& name);
    std::string toString() const;

    void display() const;
    const std::string& getName() const;

    void setCoreID(int id);
    int getCoreID() const;

    void setTotalLine(int line);
    int getTotalLine();

    void setCurrentLine(int line);
    int getCurrentLine() const;

    int getTotalLine() const;

    void setTimestamp(const std::string& ts);
    static std::string getCurrentTimestamp();
    std::string getTimestamp() const;

    void incrementLine();

private:
    std::string name;
    int coreID;
    int currentLine;
    int totalLine;
    std::string timestamp;
};

