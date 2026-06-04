#ifndef _VALUE_H
#define _VALUE_H

#include <memory>
#include <ostream>
#include <string>
#include <variant>
#include "Node.h"

class Value {
public:
    using ArrayType = std::vector<Value>;
    using Type = std::variant<
        std::monostate, 
        int, 
        double, 
        bool, 
        std::string, 
        std::shared_ptr<ArrayType>
    >;

    Value() : m_data(std::monostate {}) {}
    Value(Type val) : m_data(std::move(val)) {}

    std::variant<int, double> asNumber() const;
    auto getValue() const { return m_data; }
    void setValue(Type val) { m_data = val; }

    Value operator+(const Value& other) const;
    Value operator-(const Value& other) const;
    Value operator*(const Value& other) const;
    Value operator/(const Value& other) const;
    Value operator%(const Value& other) const;

    Value getCardinality() const;
    Value negateNum() const;

    Value logicalAnd(const Value& other) const;
    Value logicalOr(const Value& other) const;
    Value logicalNot() const;

    friend std::ostream& operator<<(std::ostream& os, const Value& val);

    Value castValue(BaseType type) const;
    int getDepth() const;

private:
    Type m_data;

    int asInt() const;
    double asFlp() const;
    bool asBool() const;
    std::string asStr() const;
};

#endif
