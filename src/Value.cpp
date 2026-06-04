#include "Value.h"
#include <stdexcept>

namespace {
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
}

std::variant<int, double> Value::asNumber() const {
    return std::visit<std::variant<int, double>>(overloaded{
        [](int val) { return val; },

        [](double val) { return val; },

        [](bool val) { return val ? 1 : 0; },

        [](const std::string& val) -> std::variant<int, double> { 
            try {
                if (val.find('.') != std::string::npos)
                    return std::stod(val);
                return std::stoi(val);
            } catch (std::invalid_argument& e) {
                throw std::runtime_error("Cannot implicitly convert string '" 
                        + val + "' into a number type");
            } catch (std::out_of_range& e) {
                throw std::runtime_error("Cannot implicitly convert string '" 
                        + val + "' into a number type - value is out of range");
            }
        },

        [](std::monostate) -> std::variant<int, double> {
            throw std::runtime_error("Cannot convert null to a number");
        }

    }, m_data);
}

Value Value::operator+(const Value& other) const {
    auto leftNum = this->asNumber();
    auto rightNum = other.asNumber();

    return std::visit([](auto&& l, auto&& r) {
        return Value(l + r);
    }, leftNum, rightNum);
}

Value Value::operator-(const Value& other) const {
    auto leftNum = this->asNumber();
    auto rightNum = other.asNumber();

    return std::visit([](auto&& l, auto&& r) {
        return Value(l - r);
    }, leftNum, rightNum);
}

Value Value::operator*(const Value& other) const {
    auto leftNum = this->asNumber();
    auto rightNum = other.asNumber();

    return std::visit([](auto&& l, auto&& r) {
        return Value(l * r);
    }, leftNum, rightNum);
}

Value Value::operator/(const Value& other) const {
    auto leftNum = this->asNumber();
    auto rightNum = other.asNumber();

    return std::visit([](auto&& l, auto&& r) {
        if (r == 0)
            throw std::runtime_error("Cannot divide by zero");

        return Value(static_cast<double>(l) / r);
    }, leftNum, rightNum);

}

std::ostream& operator<<(std::ostream& os, const Value& val) {
    std::visit([&os](const auto& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            os << "null"; 
        } else {
            os << v;     
        }

        os << "\n";
    }, val.m_data);

    return os;
}
