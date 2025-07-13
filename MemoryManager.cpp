#include "MemoryManager.h"
#include <sstream>
#include <algorithm>

MemoryManager::MemoryManager() {
    memory = std::vector<int>(FRAMES, -1);
}

bool MemoryManager::allocate(int processID) {
    int freeCount = 0, startIdx = -1;

    for (int i = 0; i < FRAMES; ++i) {
        if (memory[i] == -1) {
            if (freeCount == 0) startIdx = i;
            freeCount++;
            if (freeCount == FRAMES_PER_PROC) {
                for (int j = startIdx; j < startIdx + FRAMES_PER_PROC; ++j)
                    memory[j] = processID;
                return true;
            }
        } else {
            freeCount = 0;
        }
    }

    return false;
}

void MemoryManager::deallocate(int processID) {
    for (int i = 0; i < FRAMES; ++i) {
        if (memory[i] == processID) memory[i] = -1;
    }
}

int MemoryManager::getExternalFragmentation() const {
    int count = 0, freeCount = 0;

    for (int i = 0; i < FRAMES; ++i) {
        if (memory[i] == -1) {
            freeCount++;
        } else {
            if (freeCount > 0 && freeCount < FRAMES_PER_PROC)
                count += freeCount;
            freeCount = 0;
        }
    }

    if (freeCount > 0 && freeCount < FRAMES_PER_PROC)
        count += freeCount;

    return (count * FRAME_SIZE) / 1024; // Return in KB
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

    for (int i = 1; i < FRAMES; ++i) {
        if (memory[i] != currentPID) {
            if (currentPID != -1) {
                blocks.push_back({currentPID, startIdx, i - startIdx});
            }
            currentPID = memory[i];
            startIdx = i;
        }
    }

    // Add last block if needed
    if (currentPID != -1) {
        blocks.push_back({currentPID, startIdx, FRAMES - startIdx});
    }

    // Convert to address-based blocks
    struct DisplayBlock {
        int pid;
        int startAddress;
        int endAddress;
    };
    std::vector<DisplayBlock> display;

    for (const auto& b : blocks) {
        int startAddr = b.startFrame * FRAME_SIZE;
        int endAddr = startAddr + b.length * FRAME_SIZE;
        display.push_back({b.pid, startAddr, endAddr});
    }

    // Sort by descending startAddress
    std::sort(display.begin(), display.end(), [](const DisplayBlock& a, const DisplayBlock& b) {
        return a.startAddress > b.startAddress;
    });

    std::ostringstream oss;
    oss << "--- end --- = " << TOTAL_MEMORY << "\n";
    oss << TOTAL_MEMORY << "\n";

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