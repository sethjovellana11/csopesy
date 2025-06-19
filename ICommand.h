#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>
#include <variant>

class ICommand {
public:
    virtual std::string toString() const = 0;
    virtual ~ICommand() = default;
    virtual void execute(std::unordered_map<std::string, int32_t>& vars) = 0;
};