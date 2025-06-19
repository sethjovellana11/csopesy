#pragma once
#include "ICommand.h"
#include <vector>

class ForCommand : public ICommand {
    std::vector<std::shared_ptr<ICommand>> instructions;
    int repeats;

public:
    ForCommand(const std::vector<std::shared_ptr<ICommand>>& instructions, int repeats)
        : instructions(instructions), repeats(repeats) {}

    void execute(std::unordered_map<std::string, int32_t>& vars) override {
        for (int i = 0; i < repeats; ++i) {
            for (auto& cmd : instructions) {
                cmd->execute(vars);
            }
        }
    }

    std::string toString() const override {
    return "FOR loop x" + std::to_string(repeats) + " with " +
           std::to_string(instructions.size()) + " instructions";
    }
};