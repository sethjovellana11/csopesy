#pragma once
#include <vector>
#include <string>
#include <map>

class MemoryManager {
private:
    /* Values changed from constants to configurable values
    static const int TOTAL_MEMORY = 16384;
    static const int FRAME_SIZE = 16;
    static const int FRAMES = TOTAL_MEMORY / FRAME_SIZE;
    static const int FRAMES_PER_PROC = 4096 / FRAME_SIZE;
    */
    int total_memory, frame_size, mem_per_proc;
    int frames, frames_per_proc;

    std::vector<int> memory; 
public:
    MemoryManager(int max_overall_mem, int mem_per_frame, int mem_per_proc);

    bool allocate(int processID);
    void deallocate(int processID);
    int getExternalFragmentation() const;
    std::string asciiMemoryMap() const;
    int processesInMemory() const;
};