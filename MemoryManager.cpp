#include "MemoryManager.h"
#include <sstream>
#include <algorithm>
#include <map>
#include <iostream>

MemoryManager::MemoryManager(int max_overall_mem, int mem_per_frame, int mem_per_proc) 
    : total_memory(max_overall_mem), frame_size(mem_per_frame), mem_per_proc(mem_per_proc) {
    this->frames = this->total_memory / this->frame_size;
    this->frames_per_proc = this->mem_per_proc / frame_size;
    memory = std::vector<int>(frames, -1);
    backing_store.open(backing_store_filename, std::ios::in | std::ios::out | std::ios::binary);
}

MemoryManager::~MemoryManager() {
    if (backing_store.is_open()) {
        backing_store.close();
    }
}

bool MemoryManager::allocate(int processID, int pageID) {
    // Find free frames for the process
    for (int i = 0; i < frames; ++i) {
        if (memory[i] == -1) {  // Empty frame found
            memory[i] = processID;
            process_page_table[processID].push_back(i);
            return true;
        }
    }

    // No free frame, trigger page fault and replacement
    pageFaultHandler(processID, pageID);
    return false;
}

void MemoryManager::deallocate(int processID) {
    for (int i = 0; i < frames; ++i) {
        if (memory[i] == processID) {
            memory[i] = -1;  // Clear memory
        }
    }
}

void MemoryManager::pageFaultHandler(int processID, int pageID) {
    // Simulate page fault and replacement (using FIFO or LRU)
    int evictedPage = process_page_table[processID].front();  // Simple FIFO evict
    swapPageToBackingStore(processID, evictedPage);  // Write the page to backing store

    // Allocate the new page to memory
    loadPageFromBackingStore(processID, pageID);  // Load the page from the backing store
}

void MemoryManager::swapPageToBackingStore(int processID, int pageID) {
    // Simulate swapping the page out to the backing store (write to file)
    std::cout << "Swapping page " << pageID << " of process " << processID << " to backing store." << std::endl;
    backing_store.seekp(pageID * frame_size);
    // Here, we would write the page data (simulated by the processID)
    backing_store.write(reinterpret_cast<char*>(&processID), sizeof(processID));
}

void MemoryManager::loadPageFromBackingStore(int processID, int pageID) {
    // Simulate loading a page from the backing store (read from file)
    std::cout << "Loading page " << pageID << " of process " << processID << " from backing store." << std::endl;
    backing_store.seekg(pageID * frame_size);
    // Read the page data back into memory (simulated by assigning the process ID)
    backing_store.read(reinterpret_cast<char*>(&processID), sizeof(processID));
    for (int i = 0; i < frames; ++i) {
        if (memory[i] == -1) {
            memory[i] = processID;
            process_page_table[processID].push_back(i);
            return;
        }
    }
}

int MemoryManager::getExternalFragmentation() const {
    int count = 0, freeCount = 0;

    for (int i = 0; i < frames; ++i) {
        if (memory[i] == -1) {
            freeCount++;
        } else {
            if (freeCount > 0 && freeCount < frames_per_proc)
                count += freeCount;
            freeCount = 0;
        }
    }

    if (freeCount > 0 && freeCount < frames_per_proc)
        count += freeCount;

    return (count * frame_size) / 1024; // Return in KB
}

std::string MemoryManager::asciiMemoryMap() const {
    struct Block {
        int pid;
        int startFrame;
        int length;
    };

    std::vector<Block> blocks;
    int currentPID = memory[0];
    int startIdx = 0;

    for (int i = 1; i < frames; ++i) {
        if (memory[i] != currentPID) {
            if (currentPID != -1) {
                blocks.push_back({currentPID, startIdx, i - startIdx});
            }
            currentPID = memory[i];
            startIdx = i;
        }
    }

    if (currentPID != -1) {
        blocks.push_back({currentPID, startIdx, frames - startIdx});
    }

    struct DisplayBlock {
        int pid;
        int startAddress;
        int endAddress;
    };
    std::vector<DisplayBlock> display;

    for (const auto& b : blocks) {
        int startAddr = b.startFrame * frame_size;
        int endAddr = startAddr + b.length * frame_size;
        display.push_back({b.pid, startAddr, endAddr});
    }

    std::sort(display.begin(), display.end(), [](const DisplayBlock& a, const DisplayBlock& b) {
        return a.startAddress > b.startAddress;
    });

    std::ostringstream oss;
    oss << "--- end --- = " << total_memory << "\n";
    oss << total_memory << "\n";

    for (const auto& d : display) {
        oss << "P" << d.pid << "\n";
        oss << d.startAddress << "\n\n";
    }

    oss << "--- start --- = 0\n";
    return oss.str();
}

int MemoryManager::processesInMemory() const {
    std::map<int, bool> seen;
    for (int pid : memory) {
        if (pid != -1)
            seen[pid] = true;
    }
    return seen.size();
}
