#ifndef _ENV_H
#define _ENV_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "Value.h"
#include "Node.h"

struct VarInfo {
    TypeInfo type;
    Value value;
};

struct RefInfo {
    TypeInfo type;
    std::reference_wrapper<Value> value;
};

class Environment {
public:
    Environment(std::shared_ptr<Environment> enclosing = nullptr)
        : m_enclosing(std::move(enclosing)) {}

    void define(TypeInfo typeInfo, std::string name, Value value);
    void defineReference(TypeInfo typeInfo, std::string name, 
            const std::string& referencedName);
    void assignIdentifier(const std::string& name, Value val);
    void assignArrayOrStr(Value lValArray, Value idxVal, Value assignedVal, Identifier* idNode);
    Value get(const std::string& name) const;

private:
    void assignString(Value* valPtr, int idx, Value charVal);
    void assignArray(TypeInfo arrTypeInfo, Value lValArray, int idx,
            Value assignedVal);
    RefInfo getRefInfo(const std::string& name);
    static Value defaultInitialize(TypeInfo typeInfo);

    std::unordered_map<std::string, VarInfo> values;
    std::unordered_map<std::string, RefInfo> m_references;
    std::shared_ptr<Environment> m_enclosing;
};

#endif
