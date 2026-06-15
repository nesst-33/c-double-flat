#include "Environment.h"
#include <format>
#include <iostream>
#include <stdexcept>
#include <variant>

Value Environment::defaultInitialize(TypeInfo typeInfo) {
    if ( typeInfo.arrayDepth )
        throw std::runtime_error("Array variables have to be initialized on declaration");

    switch( typeInfo.type ) {
        case BaseType::INT:
            return Value(0);
        case BaseType::FLP:
            return Value(0.);
        case BaseType::BOOL:
            return Value(false);
        case BaseType::STR:
            return Value("");
        case BaseType::VOID:
            throw std::runtime_error("Cannot initialize void variable");
    }
}

void Environment::define(TypeInfo typeInfo, std::string name, Value value) {
    if (m_references.contains(name))
        throw std::runtime_error("Variable '" + name + "' already exists");

    BaseType type = typeInfo.type;

    // If there's an initializer:
    if ( !std::holds_alternative<std::monostate>(value.getValue()) )
        value = std::move(value.matchType(typeInfo));
    // If there's not:
    else 
        value = std::move(defaultInitialize(typeInfo)); 

    if (typeInfo.isConst) { value.qualify(); }

    auto [it, inserted] = values.try_emplace(name, typeInfo, std::move(value));
    if (!inserted)
        throw std::runtime_error("Variable '" + name + "' already exists");
}

void Environment::defineFunction(TypeInfo typeInfo, std::string name, Value value) {
    if (m_references.contains(name))
        throw std::runtime_error("Variable '" + name + "' already exists");

    auto [it, inserted] = values.try_emplace(name, typeInfo, std::move(value));
    if (!inserted)
        throw std::runtime_error("Variable '" + name + "' already exists");
}

void Environment::defineReference(TypeInfo typeInfo, std::string name,
        RefInfo refInfo) {
    if (values.contains(name))
        throw std::runtime_error("Variable " + name + " already exists");

    // Get a reference to TypeInfo and Value of the referenced variable
    if (!typeInfo.isConst && refInfo.type.isConst)
        throw std::runtime_error("Cannot make a non-const reference to a const variable");

    // Check if types matched (implicit casting is disallowed in references)
    if (typeInfo.arrayDepth != refInfo.type.arrayDepth
            || typeInfo.type != refInfo.type.type) {
        throw std::runtime_error(std::format("Cannot make a reference of type {}"
                    " to a variable of type {}", typeInfo.toString(), refInfo.type.toString()));
    }
    
    m_references.insert_or_assign(name, refInfo);
}

void Environment::assignIdentifier(const std::string& name, Value val) {
    if (auto it = m_references.find(name); it != m_references.end()) {
        TypeInfo typeInfo = it->second.type;
        if (typeInfo.isConst)
            throw std::runtime_error("Variable '" + name + "' is immutable");
        it->second.value.get() = std::move(val.matchType(typeInfo));
        return;
    }
    if (auto it = values.find(name); it != values.end()) {
        TypeInfo typeInfo = it->second.type;
        if (typeInfo.isConst)
            throw std::runtime_error("Variable '" + name + "' is immutable");
        it->second.value = std::move(val.matchType(typeInfo));
        return;
    }
    if (m_enclosing)
        return m_enclosing->assignIdentifier(name, val);

    throw std::runtime_error("Undefined variable: '" + name + "'");
}

void Environment::assignString(Value* valPtr, int idx, Value charVal) {
    charVal = charVal.castValue(BaseType::STR);
    std::string character = std::get<std::string>(charVal.getValue());
    if (character.size() != 1)
        throw std::runtime_error("You can only assign single characters to a string");

    valPtr->modifyString(idx, character[0]);
}

void Environment::assignArray(TypeInfo arrTypeInfo, Value lValArray, int idx, 
        Value assignedVal) {

    int targetDepth = lValArray.getDepth() - 1;
    int depth = assignedVal.getDepth();
    if ( targetDepth < 0 ) {
        throw std::runtime_error("Array was indexed too many times; max depth is "
                + std::to_string(arrTypeInfo.arrayDepth));
    }

    if ( depth != targetDepth ) {
        throw std::runtime_error(std::format("Array should be nested {} time(s); is nested {} time(s)",
            targetDepth, depth));
    }

    auto arrPtr = std::get<std::shared_ptr<Value::ArrayType>>(lValArray.getValue());

    try {
        arrPtr->at(idx) = assignedVal.castValue(arrTypeInfo.type);
    } catch (const std::out_of_range& e) {
        throw std::runtime_error("Index is out of range");
    }
}

void Environment::assignArrayOrStr(Value lValArray, Value idxVal, Value assignedVal, Identifier* idNode) {
    const std::string& name = idNode->getName();
    int idx = idxVal.getIndex();

    TypeInfo* typeInfoPtr = nullptr;
    Value* valPtr = nullptr;

    if (auto it = m_references.find(name); it != m_references.end()) {
        typeInfoPtr = &it->second.type;
        valPtr = &it->second.value.get();
    }
    if (auto it = values.find(name); it != values.end()) {
        typeInfoPtr = &it->second.type;
        valPtr = &it->second.value;
    }

    if (typeInfoPtr && valPtr) {
        if (typeInfoPtr->isConst) 
            throw std::runtime_error("Variable '" + name + "' is immutable");

        if (typeInfoPtr->arrayDepth == 0 && typeInfoPtr->type == BaseType::STR) 
            return assignString(valPtr, idx, assignedVal);
        else if (typeInfoPtr->arrayDepth == 0)
            throw std::runtime_error("Variable '" + name 
                    + "' doesn't support index assignment");
        else 
            return assignArray(*typeInfoPtr, lValArray, idx, assignedVal);
    }
    if (m_enclosing)
        return m_enclosing->assignArrayOrStr(lValArray, idxVal, assignedVal, idNode);

    throw std::runtime_error("Variable: '" + name + "' doesn't exist");
}


Value Environment::get(const std::string& name) const {
    if (auto it = m_references.find(name); it != m_references.end())
        return it->second.value;
    if (auto it = values.find(name); it != values.end())
        return it->second.value; 
    if (m_enclosing)
        return m_enclosing->get(name);

    throw std::runtime_error("Undefined variable: '" + name + "'");
}

RefInfo Environment::getRefInfo(const std::string& name) {
    if (auto it = m_references.find(name); it != m_references.end())
        return it->second;
    if (auto it = values.find(name); it != values.end())
        return {it->second.type, std::ref(it->second.value)};
    if (m_enclosing)
        return m_enclosing->getRefInfo(name);

    throw std::runtime_error("Undefined variable: '" + name + "'");
}

