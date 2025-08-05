#include "MemoryManager.h"
#include <sstream>
#include <algorithm>
#include <random>
#include <iostream>
#include <cmath>
#include <fstream>  

MemoryManager::MemoryManager(int max_overall_mem, int mem_per_frame, int min_mem_per_proc, int max_mem_per_proc) 
    : total_memory(max_overall_mem), frame_size(mem_per_frame), min_mem_per_proc(min_mem_per_proc), max_mem_per_proc(max_mem_per_proc) {
    this->frames = this->total_memory / this->frame_size;
    this->frames_per_proc = this->max_mem_per_proc / frame_size;
    memory = std::vector<int>(frames, -1);
}

//Used in regular paging
/*
bool MemoryManager::allocate(int processID) {
    int freeCount = 0, startIdx = -1;

    for (int i = 0; i < frames; ++i) {
        if (memory[i] == -1) {
            if (freeCount == 0) startIdx = i;
            freeCount++;
            if (freeCount == frames_per_proc) {
                for (int j = startIdx; j < startIdx + frames_per_proc; ++j)
                    memory[j] = processID;
                return true;
            }
        } else {
            freeCount = 0;
        }
    }

    return false;
}
*/

void MemoryManager::deallocate(int processID) {
    for (int i = 0; i < frames; ++i) {
        if (memory[i] == processID) {
            memory[i] = -1;
        }
    }

    // Clean up tables
    pageTable.erase(processID);
    pagesInMemory.erase(processID);
    backingStore.erase(processID);

    // Clean frame queue (remove any of processID's frames)
    std::queue<int> newQueue;
    while (!frameQueue.empty()) {
        int idx = frameQueue.front(); frameQueue.pop();
        if (memory[idx] != -1)
            newQueue.push(idx);
    }
    frameQueue = std::move(newQueue);
}

bool MemoryManager::accessPage(int processID, int pageNum) {
    // 1. Check if page is already in memory (page hit)
    if (isPageInMemory(processID, pageNum)) {
        return true;
    }

    // 2. Page fault
    totalPagedIn++;

    // 3. Try to find a free frame
    int freeFrame = -1;
    for (int i = 0; i < memory.size(); ++i) {
        if (memory[i] == -1) {
            freeFrame = i;
            break;
        }
    }

    // 4. If no free frame, evict using FIFO
    if (freeFrame == -1 && !frameQueue.empty()) {
        int victimFrame = frameQueue.front();
        frameQueue.pop();

        int victimPID = memory[victimFrame];
        int victimPage = -1;

        // Locate the evicted page
        for (const auto& [vp, pf] : pageTable[victimPID]) {
            if (pf == victimFrame) {
                victimPage = vp;
                break;
            }
        }

        // Remove mappings
        if (victimPage != -1) {
            pageTable[victimPID].erase(victimPage);
            pagesInMemory[victimPID].erase(victimPage);
            backingStore[victimPID].insert(victimPage); // Simulate backing store
        }

        totalPagedOut++;

        freeFrame = victimFrame; // Now available
    }

    // 5. Load page into selected frame
    if (freeFrame != -1) {
        memory[freeFrame] = processID;
        pageTable[processID][pageNum] = freeFrame;
        pagesInMemory[processID].insert(pageNum);
        frameQueue.push(freeFrame);

        // Simulate loading from backing store
        if (!isPageInBackingStore(processID, pageNum)) {
            backingStore[processID].insert(pageNum);
        }

        return false; // page fault handled
    }

    // Should not reach here
    std::cout << "MASSIVE ERROR" << std::endl;
    return false;
}

//old implementation
/*
bool MemoryManager::accessPage(int processID, int pageNum) {
    // Check if page is already loaded
    if (pageTable[processID].count(pageNum)) {
        return true; // Page hit
    }

    // Page fault: need to load
    int freeIdx = -1;
    for (int i = 0; i < frames; ++i) {
        if (memory[i] == -1) {
            freeIdx = i;
            break;
        }
    }

    if (freeIdx == -1) {
        // No free frame: use FIFO to evict
        if (frameQueue.empty()) return false; // Nothing to evict?

        int victimFrame = frameQueue.front();
        frameQueue.pop();

        int victimPID = memory[victimFrame];
        int victimPage = -1;

        // Find which page maps to victimFrame
        for (auto& entry : pageTable[victimPID]) {
            if (entry.second == victimFrame) {
                victimPage = entry.first;
                break;
            }
        }

        // Evict victim
        if (victimPage != -1) {
            pageTable[victimPID].erase(victimPage);
            pagesInMemory[victimPID].erase(victimPage);
            backingStore[victimPID].insert(victimPage);
            totalPagedOut++;
        }

        freeIdx = victimFrame;
    }

    // Load page into freeIdx
    memory[freeIdx] = processID;
    pageTable[processID][pageNum] = freeIdx;
    pagesInMemory[processID].insert(pageNum);
    frameQueue.push(freeIdx);
    totalPagedIn++; 

    return true;
}
*/

bool MemoryManager::isPageInMemory(int processID, int pageNum) const {
    auto it = pageTable.find(processID);
    if (it == pageTable.end()) return false;
    return it->second.count(pageNum) > 0;
}



bool MemoryManager::isPageInBackingStore(int processID, int pageNum) const {
    auto it = backingStore.find(processID);
    if (it == backingStore.end()) return false;
    return it->second.count(pageNum) > 0;
}
// NOT USED
/*
bool MemoryManager::loadPageFromBackingStore(int processID, int pageNum) {
    if (!isPageInBackingStore(processID, pageNum)) return false;

    // Find free frame or evict
    int freeIdx = -1;
    for (int i = 0; i < frames; ++i) {
        if (memory[i] == -1) {
            freeIdx = i;
            break;
        }
    }

    if (freeIdx == -1) {
        if (frameQueue.empty()) return false;

        int victimFrame = frameQueue.front(); frameQueue.pop();
        int victimPID = memory[victimFrame];
        int victimPage = -1;

        for (auto& entry : pageTable[victimPID]) {
            if (entry.second == victimFrame) {
                victimPage = entry.first;
                break;
            }
        }

        if (victimPage != -1) {
            pageTable[victimPID].erase(victimPage);
            pagesInMemory[victimPID].erase(victimPage);
            backingStore[victimPID].insert(victimPage);
            totalPagedOut++;
        }

        freeIdx = victimFrame;
    }

    // Load the page into memory
    memory[freeIdx] = processID;
    pageTable[processID][pageNum] = freeIdx;
    pagesInMemory[processID].insert(pageNum);
    backingStore[processID].erase(pageNum);
    frameQueue.push(freeIdx);
    totalPagedIn++;

    return true;
}
*/

// For paging
/*
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

    // Add last block if needed
    if (currentPID != -1) {
        blocks.push_back({currentPID, startIdx, frames - startIdx});
    }

    // Convert to address-based blocks
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

    // Sort by descending startAddress
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
*/

int MemoryManager::processesInMemory() const {
    std::map<int, bool> seen;
    for (int pid : memory) {
        if (pid != -1)
            seen[pid] = true;
    }
    return seen.size();
}

int MemoryManager::randomMemoryForProcess() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(min_mem_per_proc, max_mem_per_proc);
    return dist(gen);
}

int MemoryManager::calculatePagesRequired(int memoryRequired) const {
    return static_cast<int>(std::ceil(static_cast<double>(memoryRequired) / frame_size));
}

size_t MemoryManager::getTotalMemory() const {
    return total_memory;
}

size_t MemoryManager::getUsedMemory() const {
    size_t totalUsedMemory = 0;

    for (int pid : memory) {
        if (pid != -1) {
            auto it = processRegistry.find(pid);
            if (it != processRegistry.end() && it->second != nullptr) {
                totalUsedMemory += it->second->getMemPerPage();
            }
        }
    }

    return totalUsedMemory;
}

size_t MemoryManager::getFreeMemory() const {
    return total_memory - getUsedMemory();
}

void MemoryManager::registerProcess(Process* process) {
    processRegistry[process->getID()] = process;
}

void MemoryManager::unregisterProcess(int pid) {
    processRegistry.erase(pid);
}

bool MemoryManager::isProcessInRegistry(int processID) const {
    auto it = processRegistry.find(processID);

    if (it == processRegistry.end()) 
        return false;
    
    return true;
}

int MemoryManager::getPageIns() const {
    return totalPagedIn;
}

int MemoryManager::getPageOuts() const {
    return totalPagedOut;
}

void MemoryManager::printFrameTable() const {
    std::map<int, std::pair<int, int>> frameMap; // frameIndex → (pid, pageNum)

    // Build reverse mapping: frame → (PID, page)
    for (const auto& [pid, pages] : pageTable) {
        for (const auto& [pageNum, frameIdx] : pages) {
            frameMap[frameIdx] = {pid, pageNum};
        }
    }

    std::cout << "===== Frame Table =====\n";
    for (int i = 0; i < memory.size(); ++i) {
        if (memory[i] == -1) {
            std::cout << "Frame " << i << ": [FREE]\n";
        } else {
            auto it = frameMap.find(i);
            if (it != frameMap.end()) {
                std::cout << "Frame " << i << ": PID " << it->second.first 
                          << ", Page " << it->second.second << "\n";
            } else {
                std::cout << "Frame " << i << ": PID " << memory[i] 
                          << ", Page [Unknown]\n";
            }
        }
    }
    std::cout << "========================\n";
}

void MemoryManager::printBackingStoreToFile() const {
    std::ofstream outFile("csopesy-backingstore.txt");
    if (!outFile.is_open()) {
        std::cerr << "Error: Unable to open csopesy-backingstore.txt for writing.\n";
        return;
    }

    std::ostringstream consoleOutput;
    consoleOutput << "=== Backing Store Contents ===\n";

    for (const auto& entry : backingStore) {
        int pid = entry.first;
        const std::set<int>& pages = entry.second;

        consoleOutput << "Process " << pid << ":\n";
        outFile << "Process " << pid << ":\n";

        for (int pageNum : pages) {
            consoleOutput << "  Page " << pageNum << "\n";
            outFile << "  Page " << pageNum << "\n";
        }

        consoleOutput << "\n";
        outFile << "\n";
    }

    outFile.close();
    std::cout << consoleOutput.str();
}