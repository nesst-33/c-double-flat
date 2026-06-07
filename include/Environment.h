#ifndef _ENV_H
#define _ENV_H

#include <string>
#include <unordered_map>
#include "Value.h"
#include "Node.h"

struct VarInfo {
    TypeInfo type;
    Value value;
};

class Environment {
public:
    void define(TypeInfo type, std::string name, Value value);
    void assignIdentifier(const std::string& name, Value val);
    void assignArrayOrStr(Value lValArray, Value idxVal, Value assignedVal, Identifier* idNode);
    Value get(const std::string& name) const;
    TypeInfo getTypeInfo(const std::string& name) const;

private:
    void assignString(const std::string& name, int idx, Value charVal);
    void assignArray(TypeInfo arrTypeInfo, Value lValArray, int idx,
            Value assignedVal);
    static Value defaultInitialize(TypeInfo typeInfo);
    static Value matchType(Value value, TypeInfo typeInfo);
    std::unordered_map<std::string, VarInfo> values{};
};

#endif
