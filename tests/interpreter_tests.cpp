#include <gtest/gtest.h>
#include <limits>
#include <string>
#include <variant>
#include "Value.h"

TEST(ValueTests, NumberCasting) {
    Value strVal1 = Value(".14");
    Value strVal2 = Value("3.");
    Value strVal3 = Value("3.14");
    Value strVal4 = Value("314");
    Value strVal5 = Value("-314");
    Value strVal6 = Value(false);
    Value strVal7 = Value(true);

    EXPECT_EQ(std::get<double>(strVal1.asNumber()), 0.14);
    EXPECT_EQ(std::get<double>(strVal2.asNumber()), 3.);
    EXPECT_EQ(std::get<double>(strVal3.asNumber()), 3.14);
    EXPECT_EQ(std::get<int>(strVal4.asNumber()), 314);
    EXPECT_EQ(std::get<int>(strVal5.asNumber()), -314);
    EXPECT_EQ(std::get<int>(strVal6.asNumber()), 0);
    EXPECT_EQ(std::get<int>(strVal7.asNumber()), 1);
}

TEST(ValueTests, ThrowsOnInvalidNumberString) {
    Value strVal1 = Value("ab.cd");
    Value strVal2 = Value("ab");
    Value strVal3 = Value("9999999999999999999999");
    std::string maxDouble = std::string(402, '9') + ".999";
    Value strVal4 = Value(maxDouble);

    EXPECT_THROW(strVal1.asNumber(), std::runtime_error);
    EXPECT_THROW(strVal2.asNumber(), std::runtime_error);
    EXPECT_THROW(strVal3.asNumber(), std::runtime_error);
    EXPECT_THROW(strVal4.asNumber(), std::runtime_error);
}
