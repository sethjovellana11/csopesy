#pragma once
#include "Process.h"
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <unordered_map>
#include <set>

class MemoryManager {
private:
    /* Values changed from constants to configurable values
    static const int TOTAL_MEMORY = 16384;
    static const int FRAME_SIZE = 16;
    static const int FRAMES = TOTAL_MEMORY / FRAME_SIZE;
    static const int FRAMES_PER_PROC = 4096 / FRAME_SIZE;
    */
    int total_memory, frame_size, min_mem_per_proc, max_mem_per_proc;
    int frames, frames_per_proc;

    int totalPagedIn = 0;
    int totalPagedOut = 0;

    std::vector<int> memory; // -1 = free, else holds process ID
    std::queue<int> frameQueue; // For FIFO replacement

    // processID -> (virtualPage -> physicalFrame)
    std::map<int, std::map<int, int>> pageTable;

    // Tracks which process has which pages in memory
    std::map<int, std::set<int>> pagesInMemory;

    // Which pages are stored in backing store
    std::map<int, std::set<int>> backingStore;

    std::unordered_map<int, Process*> processRegistry;

public:
    MemoryManager(int max_overall_mem, int mem_per_frame, int min_mem_per_proc, int max_mem_per_proc);

    bool allocate(int processID);  // Optional: pre-load some pages
    void deallocate(int processID);

    bool accessPage(int processID, int pageNum); 
    int getExternalFragmentation() const;
    std::string asciiMemoryMap() const;
    int processesInMemory() const;

    //FOR DEMAND PAGING
    bool isPageInMemory(int processID, int pageNum) const;
    bool isPageInBackingStore(int processID, int pageNum) const;
    bool loadPageFromBackingStore(int processID, int pageNum);


    int calculatePagesRequired(int memoryRequired) const;
    int randomMemoryForProcess() const;

    void registerProcess(Process* process);
    void unregisterProcess(int pid);
    bool isProcessInRegistry(int processID) const;

    //VMSTATS
    size_t getTotalMemory() const;
    size_t getUsedMemory() const;
    size_t getFreeMemory() const;
    void incrementPagedIn();
    void incrementPagedOut();
    int getPageIns() const;
    int getPageOuts() const;
};