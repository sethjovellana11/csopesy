#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>
#include <variant>

// Forward declaration of Process to avoid a circular dependency with Process.h
class Process;

class ICommand {
public:
    virtual std::string toString() const = 0;
    virtual ~ICommand() = default;
    // The execute method now takes a reference to the Process executing it.
    // This allows commands to access the process's variables and add logs.
    virtual void execute(Process& process) = 0;
};
