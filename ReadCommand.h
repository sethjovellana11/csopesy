#pragma once
#include "ICommand.h"
#include "Process.h"
#include "ScreenInfo.h"
#include <sstream>
#include <iomanip>
#include <cstdint>

class ReadCommand : public ICommand {
    std::string varName;
    uint16_t address;

public:
    ReadCommand(const std::string& varName, uint16_t address)
        : varName(varName), address(address) {}

    void execute(Process& process) override {
        try {
            if (address >= process.getMemory() || address % 2 != 0) {
                process.shutdown("Access violation: READ " + varName + " at invalid address 0x" + toHex(address) + " MAX 0x" + toHex(process.getMemory()));
                goAhead = false;
                return;
            }
            else if(goAhead)
            {
                uint16_t value = process.readMemory(address);
            
            
                if (!process.getSymbolTable().insert(varName, value)) {
                    process.addLog("Symbol table full. READ value not stored in variable '" + varName + "'");
                }
                else
                    process.getVariables()[varName] = value;

                std::ostringstream oss;
                oss << "(" << ScreenInfo::getCurrentTimestamp() << ") Core:" << process.getScreenInfo().getCoreID()
                    << " \"READ " << varName << " 0x" << toHex(address) << " -> " << value << "\"";
                process.addLog(oss.str());
            }
        } catch (const std::exception& e) {
            process.shutdown("Exception during READ: " + std::string(e.what()));
        }
    }

    std::string toString() const override {
        return "READ " + varName + " 0x" + toHex(address);
    }

    std::string getName() const override {
        return "READ";
    }

private:
    std::string toHex(uint16_t value) const {
        std::ostringstream oss;
        oss << std::hex << std::uppercase << value;
        return oss.str();
    }
};