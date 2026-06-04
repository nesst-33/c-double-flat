#include <array>
#include <gtest/gtest.h>
#include <string>
#include <concepts>
#include "Value.h"

template <typename T>
requires std::integral<T> || std::floating_point<T>
void expectNum(const Value& valObj, T val) {
    EXPECT_EQ(std::get<T>(valObj.asNumber()), val); 
}

template <typename T>
void expectValue(const Value& valObj, T val) {
    EXPECT_EQ(std::get<T>(valObj.getValue()), val);
}

template <typename T, typename U>
Value addValues(T val1, U val2) {
    return Value(val1) + Value(val2);
}

template <typename T, typename U>
Value subtractValues(T val1, U val2) {
    return Value(val1) - Value(val2);
}

template <typename T, typename U>
Value multiplyValues(T val1, U val2) {
    return Value(val1) * Value(val2);
}

template <typename T, typename U>
Value divideValues(T val1, U val2) {
    return Value(val1) / Value(val2);
}

TEST(ValueTests, NumberCasting) {
    Value v1 = Value(".14");
    Value v2 = Value("3.");
    Value v3 = Value("3.14");
    Value v4 = Value("314");
    Value v5 = Value("-314");
    Value v6 = Value(false);
    Value v7 = Value(true);
    
    expectNum(v1, 0.14);
    expectNum(v2, 3.);
    expectNum(v3, 3.14);
    expectNum(v4, 314);
    expectNum(v5, -314);
    expectNum(v6, 0);
    expectNum(v7, 1);
}

TEST(ValueTests, ThrowsOnInvalidNumberString) {
    std::string maxDouble = std::string(402, '9') + ".999";

    std::array values = {
        Value("ab.cd"),
        Value("ab"),
        Value("9999999999999999999999"),
        Value(maxDouble),
        Value()
    };

    for (const auto& val : values)
        EXPECT_THROW(val.asNumber(), std::runtime_error);
}

// NOTE: I know I'm comparing floats but it seems to work without error
TEST(ValueTests, PlusOperator) {
    expectValue(addValues(1.5, "20"), 21.5);
    expectValue(addValues("-20", "2.5"), -17.5);
    expectValue(addValues("-20.5", true), -19.5);
    expectValue(addValues(false, true), 1);
    expectValue(addValues(-1, true), 0);
    expectValue(addValues("-4", "5"), 1);
    expectValue(addValues(5, 5.5), 10.5);
}

TEST(ValueTests, MinusOperator) {
    expectValue(subtractValues(1.5, "20"), -18.5);
    expectValue(subtractValues("-20", "2.5"), -22.5);
    expectValue(subtractValues("-20.5", true), -21.5);
    expectValue(subtractValues(false, true), -1);
    expectValue(subtractValues(-1, true), -2);
    expectValue(subtractValues("-4", "5"), -9);
    expectValue(subtractValues(5, 5.5), -0.5);
}

TEST(ValueTests, MultOperator) {
    expectValue(multiplyValues(1.5, "20"), 30.0);
    expectValue(multiplyValues("-20", "2.5"), -50.0);
    expectValue(multiplyValues("-20.5", true), -20.5);
    expectValue(multiplyValues(false, true), 0);
    expectValue(multiplyValues(-1, true), -1);
    expectValue(multiplyValues("-4", "5"), -20);
    expectValue(multiplyValues(5, 5.5), 27.5);
}

TEST(ValueTests, DivOperator) {
    expectValue(divideValues(3, "2"), 1.5);
    expectValue(divideValues("-20", "2.0"), -10.);
    expectValue(divideValues("-20.5", true), -20.5);
    expectValue(divideValues(false, true), 0.);
    expectValue(divideValues(-1, true), -1.);
    expectValue(divideValues("-4", "5"), -0.8);
    expectValue(divideValues(5, 10.), 0.5);
}

TEST(ValueTests, ThrowsOnZeroDivision) {
    EXPECT_THROW(divideValues(true, "0.0"), std::runtime_error);
}

TEST(ValueTests, ModOperator) {}
TEST(ValueTests, CastsValues) {}
