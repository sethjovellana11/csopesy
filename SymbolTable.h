#pragma once
#include <unordered_map>
#include <string>
#include <cstdint>
#include <optional>

class SymbolTable {
private:
    static constexpr int MAX_VARIABLES = 32;
    std::unordered_map<std::string, uint16_t> table;

public:
    bool insert(const std::string& name, uint16_t value);
    bool update(const std::string& name, uint16_t value);
    std::optional<uint16_t> get(const std::string& name) const;
    bool contains(const std::string& name) const;
    size_t size() const;
    bool isFull() const;
    void clear();
};