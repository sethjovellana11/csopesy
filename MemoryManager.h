#pragma once
#include <vector>
#include <string>
#include <map>

class MemoryManager {
private:
    static const int TOTAL_MEMORY = 16384;
    static const int FRAME_SIZE = 16;
    static const int FRAMES = TOTAL_MEMORY / FRAME_SIZE;
    static const int FRAMES_PER_PROC = 4096 / FRAME_SIZE;

    std::vector<int> memory; 
public:
    MemoryManager();

    bool allocate(int processID);
    void deallocate(int processID);
    int getExternalFragmentation() const;
    std::string asciiMemoryMap() const;
    int processesInMemory() const;
};