#pragma once
#include <vector>
#include <string>
#include <map>
#include <fstream>

class MemoryManager {
private:
    int total_memory, frame_size, mem_per_proc;
    int frames, frames_per_proc;
    std::vector<int> memory;
    std::map<int, std::vector<int>> process_page_table; // Process ID -> List of page frames

    std::string backing_store_filename = "csopesy-backing-store.txt"; // Simulate backing store file
    std::fstream backing_store; // For simulating page writes/reads from backing store

public:
    MemoryManager(int max_overall_mem, int mem_per_frame, int mem_per_proc);
    ~MemoryManager();

    bool allocate(int processID, int pageID);
    void deallocate(int processID);
    void pageFaultHandler(int processID, int pageID);
    void swapPageToBackingStore(int processID, int pageID);
    void loadPageFromBackingStore(int processID, int pageID);

    int getExternalFragmentation() const;
    std::string asciiMemoryMap() const;
    int processesInMemory() const;
};
