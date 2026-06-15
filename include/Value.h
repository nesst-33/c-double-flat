#ifndef _VALUE_H
#define _VALUE_H

#include <memory>
#include <ostream>
#include <string>
#include <variant>
#include "ICallable.h"
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
        std::shared_ptr<ArrayType>,
        std::shared_ptr<ICallable>
    >;

    Value() : m_data(std::monostate {}), m_isConst(false) {}
    Value(Type val) : m_data(std::move(val)) {}

    std::variant<int, double> asNumber() const;
    auto getValue() const { return m_data; }
    void setValue(Type val) { m_data = val; }

    Value operator+(const Value& other) const;
    Value operator-(const Value& other) const;
    Value operator*(const Value& other) const;
    Value operator/(const Value& other) const;
    Value operator%(const Value& other) const;

    Value operator<(const Value& other) const;
    Value operator>(const Value& other) const;
    Value operator<=(const Value& other) const;
    Value operator>=(const Value& other) const;
    Value operator==(const Value& other) const;
    Value operator!=(const Value& other) const;

    Value getCardinality() const;
    Value negateNum() const;

    Value logicalAnd(const Value& other) const;
    Value logicalOr(const Value& other) const;
    Value logicalNot() const;

    Value operator[](const Value& other) const;
    Value concatenate(const Value& other) const;
    Value split(const Value& other) const;
    Value intersection(const Value& other) const;
    Value append(const Value& other) const;
    Value extract(const Value& other);

    friend std::ostream& operator<<(std::ostream& os, const Value& val);

    Value castValue(BaseType type) const;
    Value matchType(TypeInfo typeInfo);
    int getDepth() const;
    int getIndex() const;

    void modifyString(int idx, char character);
    
    int asInt() const;
    double asFlp() const;
    bool asBool() const;
    std::string asStr() const;

    bool getConst() const { return m_isConst; }
    void qualify();

private:
    Type m_data;
    bool m_isConst = false;

    template <typename CompareOp>
    static Value compareStrOrNum(const Value& left, const Value& right, CompareOp op);

    template <typename CompareOp>
    static Value compareRelational(const Value& left, const Value& right, CompareOp op);

    static Value compareEquality(const Value& left, const Value& right);

    static Value areEqualArrays(const std::shared_ptr<ArrayType>& left, 
            const std::shared_ptr<ArrayType>& right);
    static Value areEqualBool(const Value& left, const Value& right);
    static bool areEqualType(const Value& left, const Value& right);

    bool requiresCasting(BaseType type) const;
    BaseType getBaseType() const;
    
    static Value multiplyArray(const std::shared_ptr<ArrayType>& arr, int mult);

};

#endif
