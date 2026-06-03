#ifndef _VALUE_H
#define _VALUE_H

#include <string>
#include <variant>

class Value {
public:
    Value() : m_data(std::monostate {}) {}
    Value(int val) : m_data(val) {}
    Value(bool val) : m_data(val) {}
    Value(double val) : m_data(val) {}
    Value(std::string val) : m_data(val) {}
    Value(const char* val) : m_data(std::string(val)) {}

    std::variant<int, double> asNumber() const;
    auto getValue() const { return m_data; }

    Value operator+(const Value& other) const;
    Value operator-(const Value& other) const;
    Value operator*(const Value& other) const;
    Value operator/(const Value& other) const;


private:
    std::variant<std::monostate, int, double, bool, std::string> m_data;
};

#endif
