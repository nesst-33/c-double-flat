#include "Value.h"
#include <algorithm>
#include <ios>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <string>
#include <utility>
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

Value Value::areEqualArrays(const std::shared_ptr<ArrayType>& left, 
        const std::shared_ptr<ArrayType>& right) {
    if (left->size() != right->size())
        return Value(false);

    for (size_t i{}; i < left->size(); i++) {
        const auto& leftArrElement = left->at(i);
        const auto& rightArrElement = right->at(i);

        if ( !areEqualType(leftArrElement, rightArrElement) )
            return Value(false);

        if ( auto* leftNestedArrPtr = std::get_if<std::shared_ptr<ArrayType>>(&leftArrElement.m_data) ) {
            const auto& rightNestedArr = std::get<std::shared_ptr<ArrayType>>(rightArrElement.m_data);

            Value areArrsEqual = areEqualArrays(*leftNestedArrPtr, rightNestedArr);

            if (!std::get<bool>(areArrsEqual.m_data))
                return areArrsEqual;
        }
        else {
            Value arePrimitivesEqual = (leftArrElement == rightArrElement);
            if (!std::get<bool>(arePrimitivesEqual.m_data))
                return arePrimitivesEqual;
        }
    }
    return Value(true);
}

Value Value::areEqualBool(const Value& left, const Value& right) {
    bool leftVal = left.asBool();
    bool rightVal = right.asBool();
    return Value(leftVal == rightVal);
}

template <typename CompareOp>
Value Value::compareStrOrNum(const Value& left, const Value& right, CompareOp op) {
    if (auto* strL = std::get_if<std::string>(&left.m_data)) {
        if (auto* strR = std::get_if<std::string>(&right.m_data)) {
            return Value(op(*strL, *strR));
        }
    }

    auto leftNum = left.asNumber();
    auto rightNum = right.asNumber();

    return std::visit([op](auto&& l, auto&& r) {
        return Value(op(l, r));
    }, leftNum, rightNum);

}

template <typename CompareOp>
Value Value::compareRelational(const Value& left, const Value& right, CompareOp op) {
    if (std::holds_alternative<std::monostate>(left.m_data)
            || std::holds_alternative<std::monostate>(right.m_data))
        throw std::runtime_error("Cannot compare void");

    if (std::holds_alternative<bool>(left.m_data)
            || std::holds_alternative<bool>(right.m_data))
        throw std::runtime_error("Cannot compare booleans");

    if (std::holds_alternative<std::shared_ptr<ArrayType>>(left.m_data)
            || std::holds_alternative<std::shared_ptr<ArrayType>>(right.m_data))
        throw std::runtime_error("Cannot compare arrays");

    return compareStrOrNum(left, right, op);
}

Value Value::compareEquality(const Value& left, const Value& right) {
    if (std::holds_alternative<std::monostate>(left.m_data)
            || std::holds_alternative<std::monostate>(right.m_data))
        throw std::runtime_error("Cannot compare void");

    if (auto* arrL = std::get_if<std::shared_ptr<ArrayType>>(&left.m_data)) {
        if (auto* arrR = std::get_if<std::shared_ptr<ArrayType>>(&right.m_data)) {
            return areEqualArrays(*arrL, *arrR);
        }
    }

    if (std::holds_alternative<bool>(left.m_data)
            || std::holds_alternative<bool>(right.m_data)) {
        return areEqualBool(left, right); 
    }

    return compareStrOrNum(left, right, [](auto&& l, auto&& r) { return l == r; });
}

Value Value::operator<(const Value& other) const {
    return compareRelational(*this, other, [](auto&& l, auto&& r) { return l < r; });
}

Value Value::operator>(const Value& other) const {
    return compareRelational(*this, other, [](auto&& l, auto&& r) { return l > r; });
}

Value Value::operator<=(const Value& other) const {
    return compareRelational(*this, other, [](auto&& l, auto&& r) { return l <= r; });
}

Value Value::operator>=(const Value& other) const {
    return compareRelational(*this, other, [](auto&& l, auto&& r) { return l >= r; });
}

Value Value::operator==(const Value& other) const {
    return compareEquality(*this, other);
}

Value Value::operator!=(const Value& other) const {
    bool result = std::get<bool>(compareEquality(*this, other).m_data);
    return Value(!result);
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

        [](const std::shared_ptr<ArrayType>&) -> bool {
            throw std::runtime_error("Cannot convert array to boolean");
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

int Value::getIndex() const {
    auto* idxPtr = std::get_if<int>(&m_data);
    if (!idxPtr)
        throw std::runtime_error("Indexes have to be of type int");

    return *idxPtr;
}

Value Value::operator[](const Value& other) const {
    int idx = other.getIndex();

    return std::visit(overloaded{
        [idx](const std::shared_ptr<ArrayType>& arr) -> Value {
            try {
                return arr->at(idx);
            } catch (const std::out_of_range& e) {
                throw std::runtime_error("Index is out of range");
            }
        },

        [idx](const std::string& str) -> Value {
            try {
                char letter = str.at(idx);
                return Value(std::string(1, letter));
            } catch (const std::out_of_range& e) {
                throw std::runtime_error("Index is out of range");
            }
        },

        [idx](int num) -> Value { return Value(std::to_string(num))[Value(idx)]; },
        [idx](double num) -> Value { return Value(std::to_string(num))[Value(idx)]; },

        [](auto&&) -> Value {
            throw std::runtime_error("This type cannot be indexed");
        } 

    }, this->m_data);
}

Value Value::concatenate(const Value& other) const {
    return std::visit(overloaded{
        [](const std::shared_ptr<ArrayType>& arrL,
                const std::shared_ptr<ArrayType>& arrR) {
            auto concatenated = std::make_shared<ArrayType>();
            concatenated->reserve(arrL->size() + arrR->size());

            concatenated->insert(concatenated->end(), arrL->begin(), arrL->end());
            concatenated->insert(concatenated->end(), arrR->begin(), arrR->end());

            return Value(std::move(concatenated));
        },

        [](const std::string& strL, const std::string& strR) {
            return Value(strL + strR);
        },

        [](auto&&, auto&&) -> Value {
            throw std::runtime_error("You can only concatenate arrays or strings");
        }
    }, this->m_data, other.m_data);
}



Value Value::split(const Value& other) const {
    int idx = other.getIndex();

    return std::visit(overloaded{
        [idx](const std::shared_ptr<ArrayType>& arr) {
            if (idx >= arr->size())
                throw std::runtime_error("Index is out of range");
                
            auto tail = std::make_shared<ArrayType>(
                    arr->begin() + idx, arr->end()
                    );

            return Value(std::move(tail)); 
        },

        [idx](const std::string& str) {
            if (idx >= str.size())
                throw std::runtime_error("Index is out of range");

            auto tail = std::string(
                    str.begin() + idx, str.end()
                    );
            return Value(std::move(tail));
        },

        [](auto&&) -> Value {
            throw std::runtime_error("You can only split arrays or strings");
        }

    }, this->m_data);
}
