#pragma once
#include "ICommand.h"
#include "DeclareCommand.h"
#include "AddCommand.h"
#include "SubtractCommand.h"
#include "PrintCommand.h"
#include "SleepCommand.h"
#include "ForCommand.h"

#include <vector>
#include <string>
#include <random>
#include <memory>
#include <unordered_set>
#include <unordered_map>

class InstructionGenerator {
public:
    InstructionGenerator(int maxDepth = 3)
        : maxDepth(maxDepth), rng(rd()) {}

    std::vector<std::shared_ptr<ICommand>> generateInstructions(int count, int currentDepth = 0);

private:
    std::random_device rd;
    std::mt19937 rng;
    int maxDepth;
    std::unordered_set<std::string> declaredVars;
    std::vector<std::string> varPool = {"a", "b", "c", "d", "e"};

    std::string getRandomVar(bool mustExist = false);
    uint16_t getRandomValue();
    std::shared_ptr<ICommand> generateRandomInstruction(int currentDepth);
};