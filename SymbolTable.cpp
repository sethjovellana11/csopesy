#include "SymbolTable.h"

bool SymbolTable::insert(const std::string& name, uint16_t value) {
    if (table.size() >= MAX_VARIABLES && !contains(name))
        return false;
    table[name] = value;
    return true;
}

bool SymbolTable::update(const std::string& name, uint16_t value) {
    if (!contains(name)) return false;
    table[name] = value;
    return true;
}

std::optional<uint16_t> SymbolTable::get(const std::string& name) const {
    auto it = table.find(name);
    if (it != table.end())
        return it->second;
    return std::nullopt;
}

bool SymbolTable::contains(const std::string& name) const {
    return table.find(name) != table.end();
}

size_t SymbolTable::size() const {
    return table.size();
}

bool SymbolTable::isFull() const {
    return table.size() >= MAX_VARIABLES;
}

void SymbolTable::clear() {
    table.clear();
}
