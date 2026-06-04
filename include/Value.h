#ifndef _VALUE_H
#define _VALUE_H

#include <ostream>
#include <string>
#include <variant>

class Value {
public:
    using Type = std::variant<std::monostate, int, double, bool, std::string>;

    Value() : m_data(std::monostate {}) {}
    Value(Type val) : m_data(std::move(val)) {}

    std::variant<int, double> asNumber() const;
    auto getValue() const { return m_data; }
    void setValue(Type val) { m_data = val; }

    Value operator+(const Value& other) const;
    Value operator-(const Value& other) const;
    Value operator*(const Value& other) const;
    Value operator/(const Value& other) const;

    friend std::ostream& operator<<(std::ostream& os, const Value& val);

private:
    Type m_data;
};

#endif
