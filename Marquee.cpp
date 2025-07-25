#include "Marquee.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

// Helper function for non-blocking input on Linux
#ifndef _WIN32
int _kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

char _getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror ("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror ("tcsetattr ~ICANON");
    return (buf);
}
#endif

bool Marquee::isRunning() const {
    return running;
}

Marquee::Marquee(const std::string& text, int width, int height)
    : text(text), width(width), height(height), running(false), fps(30),
      x(0), y(height - 1), dx(1), dy(-1) {}

void Marquee::start() {
    if(running) return;
    running = true;
    inputThread = std::thread(&Marquee::handleInput, this);
    marqueeThread = std::thread(&Marquee::run, this);
}

void Marquee::stop() {
    running = false;
    cv.notify_one(); 
    if (marqueeThread.joinable())
        marqueeThread.join();
    if (inputThread.joinable())
        inputThread.join();
}

void Marquee::setFPS(int newFPS) {
    fps = newFPS;
}

int Marquee::getFPS() const {
    return fps;
}

void Marquee::run() {
    while (running) {  
        draw();
        updatePosition();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps));
    }
}

void Marquee::draw() {
    std::string inputCopy;
    {
        std::lock_guard<std::mutex> lock(inputMutex);
        inputCopy = currentInput;
    }
    #ifdef _WIN32
        system("cls");
    #else
        std::cout << "\033[2J\033[1;1H";
    #endif

    std::cout << "******************************\nWelcome to the Marquee Console!\n******************************\n";

    for (int row = 0; row < height; ++row) {
        if (row == y) {
            for (int col = 0; col < width; ++col) {
                if (col == x && x + text.length() <= width) {
                    std::cout << text;
                    col += text.length() - 1;
                } else {
                    std::cout << " ";
                }
            }
        } else {
            std::cout << std::string(width, ' ');
        }
        std::cout << "\n";
    }

    std::cout << "\nType 'exit' to return, 'marquee-fps <value>' to change speed,\n or marquee-text <text> to change marquee text\n";
    std::cout << "> " << inputCopy << std::flush; 
}

void Marquee::updatePosition() {
    // Bounce horizontally
    if (x + text.length() >= width) {
        x = width - text.length();
        dx = -1;
    } else if (x <= 0) {
        x = 0;
        dx = 1;
    }

    // Bounce vertically
    if (y >= height - 1) {
        y = height - 1;
        dy = -1;
    } else if (y <= 0) {
        y = 0;
        dy = 1;
    }

    x += dx;
    y += dy;
}

void Marquee::handleInput() {
    while (running) {
        if (_kbhit()) {
            char ch = _getch();
            std::lock_guard<std::mutex> lock(inputMutex);

            if (ch == '\n' || ch == '\r') {
                std::string input = currentInput;

                if (input == "exit") {
                    running = false;
                } else if (input.rfind("marquee-fps ", 0) == 0) {
                    try {
                        int newFps = std::stoi(input.substr(12));
                        setFPS(newFps);
                    } catch (...) {}
                } else if (input.rfind("marquee-text ", 0) == 0){
                    text = input.substr(13);
                }
                currentInput.clear();
            } else if (ch == 127 || ch == 8) { // Backspace
                if (!currentInput.empty()) {
                    currentInput.pop_back();
                }
            } else if (isprint(ch)){
                currentInput += ch;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // prevent busy-waiting
    }
}
