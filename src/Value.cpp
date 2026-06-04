#include "Value.h"
#include <algorithm>
#include <ios>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <string>
#include <variant>

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
                throw std::runtime_error("Cannot convert string '" 
                        + val + "' into a number type");
            } catch (std::out_of_range& e) {
                throw std::runtime_error("Cannot convert string '" 
                        + val + "' into a number type - value is out of range");
            }
        },

        [](const auto& unsupported) -> std::variant<int, double> {
            throw std::runtime_error("Cannot convert this type to a number");
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
        if (!r)
            throw std::runtime_error("Cannot divide by zero");

        return Value(static_cast<double>(l) / r);
    }, leftNum, rightNum);

}

Value Value::operator%(const Value& other) const {
    auto leftNum = this->asNumber();
    auto rightNum = other.asNumber();

    return std::visit([](auto&&l , auto&& r) {
        if (!r)
            throw std::runtime_error("Modulus divisor cannot be zero");

        return Value(std::fmod(l, r));
    }, leftNum, rightNum);
}

std::ostream& operator<<(std::ostream& os, const Value& val) {
    os << std::boolalpha;
    std::visit(overloaded{
        [&os](std::monostate) { os << "void"; },

        [&os](const std::shared_ptr<Value::ArrayType>& arr) {
            os << "[";
            for (size_t i{}; i < arr->size() - 1; i++)
                os << arr->at(i) << ", "; 
            os << arr->back();
            os << "]";
        },

        [&os](const auto& v) { os << v; }
    }, val.m_data);
    os << std::noboolalpha;

    return os;
}

int Value::asInt() const { 
    return std::visit( [](auto v) {
        return static_cast<int>(v);
    }, this->asNumber());
}

double Value::asFlp() const {
    return std::visit( [](auto v) {
        return static_cast<double>(v);
    }, this->asNumber());
}

bool Value::asBool() const {
    return std::visit(overloaded{
        [](const std::string& val) {
            return !val.empty();
        },
        
        [](std::monostate) -> bool {
            throw std::runtime_error("Cannot convert void to boolean");
        },

        [](bool val) { return val; },

        [](auto val) {
            return val != 0;
        }
    }, m_data);
}

std::string Value::asStr() const {
    return std::visit(overloaded{
        [](std::monostate) -> std::string {
            throw std::runtime_error("Cannot convert void to string");
        },

        [](std::shared_ptr<ArrayType>) -> std::string {
            throw std::runtime_error("Cannot convert array to string");
        },

        [](std::string val) { return val; },

        [](bool val) -> std::string {
            return val ? "true" : "false";
        },


        [](auto val) {
            return std::to_string(val);
        }
    }, m_data);
}

Value Value::castValue(BaseType type) const {
    if (const auto* arr = std::get_if<std::shared_ptr<ArrayType>>(&m_data)) {
        auto newArr = std::make_shared<ArrayType>();
        for (const auto& val : **arr) {
            newArr->push_back(val.castValue(type));
        }
        return Value(std::move(newArr));
    }
    else {
        switch(type) {
            case BaseType::INT:
                return Value(this->asInt());
            case BaseType::FLP:
                return Value(this->asFlp());
            case BaseType::BOOL:
                return Value(this->asBool());
            case BaseType::STR:
                return Value(this->asStr());
            default:
                throw std::runtime_error("Cannot cast void values");
        }
    }
}

int Value::getDepth() const {
    if (auto* arrPtr = std::get_if<std::shared_ptr<ArrayType>>(&m_data)) {
        const auto& arr = **arrPtr;
        if (arr.empty())
            return 1;

        int expectedDepth{-1}; 
        for (const auto& element: arr) {
            int currentDepth = element.getDepth();

            if (expectedDepth == -1)
                expectedDepth = currentDepth;
            else if (expectedDepth != currentDepth)
                throw std::runtime_error("Array nesting has to be consistent");
        }

        return expectedDepth + 1;
    }
    else
        return 0;
}

Value Value::getCardinality() const {
    return std::visit(overloaded{
            [](const std::shared_ptr<ArrayType>& arr) {
                int size = arr->size();
                return Value(size);
            },
            [](const std::string& str) {
                int size = str.size();
                return Value(size);
            },
            [](auto val) -> Value {
                throw std::runtime_error("Cardinality can be only used with string or array types");
            }
        }, m_data);
}

Value Value::negateNum() const {
    auto num = this->asNumber();
    return std::visit( [](auto&& val) { return Value(-val); }, num);
}

Value Value::logicalAnd(const Value& other) const {
    bool leftFactor = this->asBool();
    bool rightFactor = other.asBool();

    return Value(leftFactor && rightFactor);
}

Value Value::logicalOr(const Value& other) const {
    bool leftFactor = this->asBool();
    bool rightFactor = other.asBool();

    return Value(leftFactor || rightFactor);
}

Value Value::logicalNot() const {
    return Value(!this->asBool()); 
}
