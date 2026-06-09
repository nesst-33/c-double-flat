#include "Environment.h"
#include <stdexcept>
#include <variant>

// QUESTION: should this be a Value class method?
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

Value Environment::matchType(Value value, TypeInfo typeInfo) {
    value = std::move(value.castValue(typeInfo.type));
    int targetDepth = typeInfo.arrayDepth;
    int depth = value.getDepth();
    if ( depth != targetDepth ) {
        throw std::runtime_error(std::format("Array should be nested {} time(s); is nested {} time(s)",
                targetDepth, depth));
    }
    return value; 
}

void Environment::define(TypeInfo typeInfo, std::string name, Value value) {
    BaseType type = typeInfo.type;

    // If there's an initializer:
    if ( !std::holds_alternative<std::monostate>(value.getValue()) ) {
        value = std::move(matchType(value, typeInfo));
    }
    // If there's not:
    else {
        value = std::move(defaultInitialize(typeInfo));
    }

    // redefinition is allowed because it's better for a REPL
    values[name] = VarInfo{typeInfo, std::move(value)};
}

void Environment::assignIdentifier(const std::string& name, Value val) {
    TypeInfo typeInfo = values.at(name).type;
    if (typeInfo.isConst)
        throw std::runtime_error("Variable '" + name + "' is immutable");

    auto it = values.find(name);
    if (it != values.end()) {
        it->second.value = std::move(matchType(val, typeInfo));
        return;
    }
    if (m_enclosing)
        return m_enclosing->assignIdentifier(name, val);

    throw std::runtime_error("Undefined variable: '" + name + "'");
}

void Environment::assignString(auto it, int idx, Value charVal) {
    
    charVal = charVal.castValue(BaseType::STR);
    std::string character = std::get<std::string>(charVal.getValue());
    if (character.size() != 1)
        throw std::runtime_error("You can only assign single characters to a string");

    it->second.value.modifyString(idx, character[0]);
}

void Environment::assignArray(TypeInfo arrTypeInfo, Value lValArray, int idx, 
        Value assignedVal) {

    int targetDepth = lValArray.getDepth() - 1;
    int depth = assignedVal.getDepth();
    if ( targetDepth <= 0 ) {
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

TypeInfo Environment::getTypeInfo(const std::string& name) const {
    try {
        return values.at(name).type;
    }
    catch (const std::out_of_range& e) {
        throw std::runtime_error("Undefined variable: '" + name + "'");
    }
}

void Environment::assignArrayOrStr(Value lValArray, Value idxVal, Value assignedVal, Identifier* idNode) {
    const std::string& name = idNode->getName();
    TypeInfo arrTypeInfo = getTypeInfo(name);

    if (arrTypeInfo.isConst) 
        throw std::runtime_error("Variable '" + name + "' is immutable");

    int idx = idxVal.getIndex();

    auto it = values.find(name);
    if (it != values.end()) {
        if (arrTypeInfo.arrayDepth == 0 && arrTypeInfo.type == BaseType::STR) 
            return assignString(it, idx, assignedVal);
        else if (arrTypeInfo.arrayDepth == 0)
            throw std::runtime_error("Variable '" + name 
                    + "' doesn't support index assignment");
        else 
            return assignArray(arrTypeInfo, lValArray, idx, assignedVal);
    }
    if (m_enclosing)
        return m_enclosing->assignArrayOrStr(lValArray, idxVal, assignedVal, idNode);

    throw std::runtime_error("Variable: '" + name + "' doesn't exist");
}


Value Environment::get(const std::string& name) const {
    auto it = values.find(name);
    if (it != values.end())
        return it->second.value; 
    if (m_enclosing)
        return m_enclosing->get(name);

    throw std::runtime_error("Undefined variable: '" + name + "'");
}

