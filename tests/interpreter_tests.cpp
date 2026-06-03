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

TEST(ValueTests, PlusOperator) {
    expectValue(addValues(1.5, "20"), 21.5);

}
