#include "InstructionGenerator.h"
#include <random>

std::string InstructionGenerator::getRandomVar(bool mustExist) {
    std::uniform_int_distribution<> dist(0, varPool.size() - 1);

    if (mustExist) {
        if (declaredVars.empty()) {
            // Fallback to declare a new one
            return getRandomVar(false);
        }

        std::string chosen;
        do {
            chosen = varPool[dist(rng)];
        } while (declaredVars.find(chosen) == declaredVars.end());

        return chosen;
    } else {
        std::string chosen = varPool[dist(rng)];
        declaredVars.insert(chosen);
        return chosen;
    }
}

uint16_t InstructionGenerator::getRandomValue() {
    static std::uniform_int_distribution<uint16_t> valDist(1, 100);
    return valDist(rng);
}

std::shared_ptr<ICommand> InstructionGenerator::generateRandomInstruction(int currentDepth) {
    std::uniform_int_distribution<int> typeDist(0, currentDepth < maxDepth ? 7 : 6);
    int type = typeDist(rng);

    switch (type) {
        case 0: { // DECLARE
            std::string var = getRandomVar(false); // Creates a new declared var
            return std::make_shared<DeclareCommand>(var, getRandomValue());
        }

        case 1: { // ADD
            std::string target = getRandomVar(false);       // New variable to assign result
            std::string op1 = getRandomVar(true);           // Must already exist
            std::string op2 = getRandomVar(true);
            return std::make_shared<AddCommand>(target, op1, op2);
        
        }

        case 2: { // SUBTRACT
            std::string target = getRandomVar(false);
            std::string op1 = getRandomVar(true);
            std::string op2 = getRandomVar(true);
            return std::make_shared<SubtractCommand>(target, op1, op2);
        }

        case 3: { // PRINT
            std::string var = getRandomVar(true);
            return std::make_shared<PrintCommand>("Print value of " + var + ": +" + var);
        }

        case 4: { // SLEEP
            std::uniform_int_distribution<uint8_t> ticks(1, 5);
            return std::make_shared<SleepCommand>(ticks(rng));
        }

        case 5: { // FOR loop
            std::uniform_int_distribution<int> repeatDist(1, 3);
            std::uniform_int_distribution<int> lenDist(1, 3);
            int repeatCount = repeatDist(rng);
            int bodyLength = lenDist(rng);
            auto body = generateInstructions(bodyLength, currentDepth + 1);
            return std::make_shared<ForCommand>(body, repeatCount);
        }
        case 6: { // READ
            std::string var = getRandomVar(false);  // store into a new variable
            std::uniform_int_distribution<uint16_t> addrDist(0x0000, 0xFFFF);
            uint16_t addr = addrDist(rng) & 0xFFFE;  // align to even byte for 2-byte read
            return std::make_shared<ReadCommand>(var, addr);
        }

        case 7: { // WRITE
            std::uniform_int_distribution<uint16_t> addrDist(0x0000, 0xFFFF);
            uint16_t addr = addrDist(rng) & 0xFFFE;  // ensure 2-byte aligned
            uint16_t val = getRandomValue();
            return std::make_shared<WriteCommand>(addr, val);
        }

        default:
            return nullptr;
    }
}

std::vector<std::shared_ptr<ICommand>> InstructionGenerator::generateInstructions(int count, int currentDepth) {
    std::vector<std::shared_ptr<ICommand>> result;
    for (int i = 0; i < count; ++i) {
        result.push_back(generateRandomInstruction(currentDepth));
    }
    return result;
}
