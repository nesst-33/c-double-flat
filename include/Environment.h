#ifndef _ENV_H
#define _ENV_H

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <iostream>
#include "Value.h"
#include "Node.h"

struct VarInfo {
    TypeInfo type;
    Value value;
};

class Environment {
public:
    void define(TypeInfo type, std::string name, Value value) {
        // redefinition is allowed because it's better for a REPL
        values[name] = VarInfo{type, std::move(value)};
    }

    Value get(const std::string& name) {
        try {
            return values.at(name).value;
        }
        catch (std::out_of_range& e) {
            throw std::runtime_error("Undefined variable: '" + name + "'");
        }
    }


private:
    std::unordered_map<std::string, VarInfo> values{};
};

#endif
