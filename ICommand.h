#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>
#include <variant>

class Process;

class ICommand {
public:
    virtual std::string toString() const = 0;
    virtual ~ICommand() = default;
    virtual std::string getName() const = 0;
    // The execute method now takes a reference to the Process executing it.
    // This allows commands to interact with the process's state, like its variables and logs.
    virtual void execute(Process& process) = 0;

    bool goAhead = true;
};
