#pragma once
#include "ICommand.h"
#include "Process.h"
#include "ScreenInfo.h"
#include <sstream>
#include <iomanip>
#include <cstdint>

class WriteCommand : public ICommand {
    uint16_t address;
    uint16_t value;

public:
    WriteCommand(uint16_t address, uint16_t value)
        : address(address), value(value) {}

    void execute(Process& process) override {
        try {
            if (address >= process.getMemory()) {
                process.shutdown("Access violation: WRITE to invalid address 0x" + toHex(address) + " MAX " + toHex(process.getMemory()));
                return;
            }

            process.writeMemory(address, value);

            std::ostringstream oss;
            oss << "(" << ScreenInfo::getCurrentTimestamp() << ") Core:" << process.getScreenInfo().getCoreID()
                << " \"WRITE 0x" << toHex(address) << " " << value << "\"";
            process.addLog(oss.str());
        } catch (const std::exception& e) {
            process.shutdown("Exception during WRITE: " + std::string(e.what()));
        }
    }

    std::string toString() const override {
        return "WRITE 0x" + toHex(address) + " " + std::to_string(value);
    }

private:
    std::string toHex(uint16_t value) const {
        std::ostringstream oss;
        oss << std::hex << std::uppercase << value;
        return oss.str();
    }
};
