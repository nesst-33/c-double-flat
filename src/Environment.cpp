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

void Environment::assign(const std::string& name, Value val) {
    try {
        TypeInfo typeInfo = values.at(name).type;
        if (typeInfo.isConst) {
            throw std::runtime_error("Variable '" + name + "' is immutable");
        }
        
        values.at(name).value = std::move(matchType(val, typeInfo));
    }
    catch (const std::out_of_range& e) {
        throw std::runtime_error("Undefined variable: '" + name + "'");
    }
}

Value Environment::get(const std::string& name) const {
    try {
        return values.at(name).value;
    }
    catch (const std::out_of_range& e) {
        throw std::runtime_error("Undefined variable: '" + name + "'");
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
