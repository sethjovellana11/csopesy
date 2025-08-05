#include "InstructionGenerator.h"
#include <random>
#include <algorithm> 

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

std::vector<std::shared_ptr<ICommand>> InstructionGenerator::generate(const std::string& input) {
    std::vector<std::shared_ptr<ICommand>> commands;
    std::stringstream ss(input);
    std::string token;
    int instructionCount = 0;

    while (std::getline(ss, token, ';')) {
        ++instructionCount;
        if (instructionCount > 50) {
            std::cout << "invalid command: more than 50 instructions" << std::endl;
            return {};
        }

        std::istringstream line(token);
        std::string command;
        line >> command;

        if (command == "DECLARE") {
            std::string var;
            uint16_t value;
            line >> var >> value;
            if (var.empty()) {
                std::cout << "invalid DECLARE syntax" << std::endl;
                return {};
            }
            commands.push_back(std::make_shared<DeclareCommand>(var, value));
            declaredVars.insert(var);
        }
        else if (command == "ADD") {
            std::string target, op1, op2;
            line >> target >> op1 >> op2;
            if (target.empty() || op1.empty() || op2.empty()) {
                std::cout << "invalid ADD syntax" << std::endl;
                return {};
            }
            commands.push_back(std::make_shared<AddCommand>(target, op1, op2));
            declaredVars.insert(target);
        }
        else if (command == "SUB") {
            std::string target, op1, op2;
            line >> target >> op1 >> op2;
            if (target.empty() || op1.empty() || op2.empty()) {
                std::cout << "invalid SUB syntax" << std::endl;
                return {};
            }
            commands.push_back(std::make_shared<SubtractCommand>(target, op1, op2));
            declaredVars.insert(target);
        }
        else if (command == "WRITE") {
            std::string addrStr, valStr;
            line >> addrStr >> valStr;
            if (addrStr.empty() || valStr.empty()) {
                std::cout << "invalid WRITE syntax" << std::endl;
                return {};
            }

            uint16_t address = std::stoi(addrStr, nullptr, 16);
            if (std::isdigit(valStr[0])) {
                uint16_t val = std::stoi(valStr);
                commands.push_back(std::make_shared<WriteCommand>(address, val));
            } else {
                commands.push_back(std::make_shared<WriteCommand>(address, valStr));
            }
        }
        else if (command == "READ") {
            std::string var, addrStr;
            line >> var >> addrStr;
            if (var.empty() || addrStr.empty()) {
                std::cout << "invalid READ syntax" << std::endl;
                return {};
            }

            uint16_t address = std::stoi(addrStr, nullptr, 16);
            commands.push_back(std::make_shared<ReadCommand>(var, address));
            declaredVars.insert(var);
        }
        else if (command == "PRINT") {
            std::string args;
            std::getline(line, args); 
            args.erase(0, args.find_first_not_of(" \t")); 

            if (args.front() == '(' && args.back() == ')') 
            {
                args = args.substr(1, args.size() - 2); 
            }
            
            if (args.find('+') != std::string::npos && args.find('"') != std::string::npos) {
                // Parse PRINT("Prefix" + varName)
                size_t quoteStart = args.find('"');
                size_t quoteEnd = args.find('"', quoteStart + 1);
                size_t plusPos = args.find('+');

                std::string prefix = args.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                std::string varName = args.substr(plusPos + 1);
                varName.erase(0, varName.find_first_not_of(" \t"));  // trim left

                commands.push_back(std::make_shared<PrintCommand>(prefix, varName));
            } else {
                
                commands.push_back(std::make_shared<PrintCommand>(args));
            }
        }

        else if (command == "SLEEP") {
            int ticks;
            line >> ticks;
            if (ticks <= 0) {
                std::cout << "invalid SLEEP syntax" << std::endl;
                return {};
            }
            commands.push_back(std::make_shared<SleepCommand>(ticks));
        }
        else {
            std::cout << "invalid command: " << command << std::endl;
            return {};
        }
    }

    if (instructionCount < 1) {
        std::cout << "invalid command: less than 1 instruction" << std::endl;
        return {};
    }

    return commands;
}


std::shared_ptr<ICommand> InstructionGenerator::generateRandomInstruction(int currentDepth, int mem) {
    std::uniform_int_distribution<int> typeDist(0, 7);
    int type = typeDist(rng);
    uint16_t maxAddr = static_cast<uint16_t>(std::max(2, mem - 2));

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
            return std::make_shared<PrintCommand>("", var);
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
            auto body = generateInstructions(bodyLength, currentDepth + 1, mem);
            return std::make_shared<ForCommand>(body, repeatCount);
        }
       case 6: { // READ
            std::string var = getRandomVar(false);
            uint16_t maxSlot = static_cast<uint16_t>((mem / 2) - 1);
            //std::cout << mem << std::endl;
            std::uniform_int_distribution<uint16_t> addrDist(0, maxSlot);
            uint16_t addr = addrDist(rng) * 2;
            return std::make_shared<ReadCommand>(var, addr);
        }

        case 7: { // WRITE
            uint16_t maxSlot = static_cast<uint16_t>((mem / 2) - 1);
            //std::cout << mem << std::endl;
            std::uniform_int_distribution<uint16_t> addrDist(0, maxSlot);
            uint16_t addr = addrDist(rng) * 2;
            uint16_t val = getRandomValue();
            return std::make_shared<WriteCommand>(addr, val);
        }

        default:
            return nullptr;
    }
}

std::vector<std::shared_ptr<ICommand>> InstructionGenerator::generateInstructions(int count, int currentDepth, int mem) {
    std::vector<std::shared_ptr<ICommand>> result;
    for (int i = 0; i < count; ++i) {
        result.push_back(generateRandomInstruction(currentDepth, mem));
    }
    return result;
}