#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <mutex>

class Marquee {
public:
    Marquee(const std::string& text, int width, int height);
    void start();
    void stop();
    void setFPS(int fps);
    int getFPS() const;
    bool isRunning() const;

private:
    void run();
    void draw();
    void updatePosition();
    void handleInput();

    std::string text;
    int textColor = 7;
    int width, height;
    std::atomic<bool> running;
    int fps;
    int x, y, dx, dy;

    std::string currentInput;
    std::mutex inputMutex;

    std::thread marqueeThread;
    std::thread inputThread;
};

