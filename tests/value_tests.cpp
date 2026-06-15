#include <array>
#include <gtest/gtest.h>
#include <initializer_list>
#include <memory>
#include <stdexcept>
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

template <typename T, typename U>
Value modValues(T val1, U val2) {
    return Value(val1) % Value(val2);
}

template <typename T, typename U>
Value lessThan(T val1, U val2) {
    return Value(val1) < Value(val2);
}

template <typename T, typename U>
Value greaterThan(T val1, U val2) {
    return Value(val1) > Value(val2);
}

template <typename T, typename U>
Value lessEq(T val1, U val2) {
    return Value(val1) <= Value(val2);
}

template <typename T, typename U>
Value greaterEq(T val1, U val2) {
    return Value(val1) >= Value(val2);
}

template <typename T, typename U>
Value equalTo(T val1, U val2) {
    return Value(val1) == Value(val2);
}

template <typename T, typename U>
Value notEqTo(T val1, U val2) {
    return Value(val1) != Value(val2);
}

template <typename T, typename U>
Value andOp(T val1, U val2) {
    return Value(val1).logicalAnd(Value(val2));
}

template <typename T, typename U>
Value orOp(T val1, U val2) {
    return Value(val1).logicalOr(Value(val2));
}

template <typename T>
Value notOp(T val1) {
    return Value(val1).logicalNot();
}

template <typename T, typename U>
Value concatValues(T val1, U val2) {
    return Value(val1).concatenate(Value(val2));
}

template <typename T, typename U>
Value splitValue(T val1, U val2) {
    return Value(val1).split(Value(val2));
}

template <typename T, typename U>
Value intersectValues(T val1, U val2) {
    return Value(val1).intersection(Value(val2));
}

template <typename T>
Value getArrayVal(std::initializer_list<T> values) {
    Value::ArrayType arr;
    arr.reserve(values.size());
    for (auto value : values) 
        arr.emplace_back(value);
    return std::move(Value(std::make_shared<Value::ArrayType>(arr)));
}

template <typename T, typename U>
Value performArrayOp(std::initializer_list<T> vals1, std::initializer_list<U> vals2,
        auto op) {
    return op(getArrayVal(vals1), getArrayVal(vals2));
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

    auto subOp = [](const Value& l, const Value& r){ return l - r; }; 
    Value result = performArrayOp({1, 2, 1, 3, 2}, {"1.", "2."}, subOp);
    Value expected = getArrayVal({3});
    EXPECT_TRUE((result == expected).asBool()); 
    
    // [[1, 2], [3, 4], [1, 2]]
    Value nestedArr1 = getArrayVal({getArrayVal({1, 2}), getArrayVal({3, 4}),
            getArrayVal({1, 2})});

    // [["1.0", "2."]]
    Value nestedArr2 = getArrayVal({getArrayVal({"1.0", "2."})});

    // [[3, 4]]
    Value nestedExpected = getArrayVal({getArrayVal({3, 4})});
    expectValue(nestedArr1 - nestedArr2 == nestedExpected, true);
}

TEST(ValueTests, MultOperator) {
    expectValue(multiplyValues(1.5, "20"), 30.0);
    expectValue(multiplyValues("-20", "2.5"), -50.0);
    expectValue(multiplyValues("-20.5", true), -20.5);
    expectValue(multiplyValues(false, true), 0);
    expectValue(multiplyValues(-1, true), -1);
    expectValue(multiplyValues("-4", "5"), -20);
    expectValue(multiplyValues(5, 5.5), 27.5);

    auto multOp = [](const Value& l, const Value& r) { return l * r; };
    Value result = getArrayVal({2, 3}) * Value(3);
    Value expected = getArrayVal({2, 3, 2, 3, 2, 3});
    EXPECT_TRUE((result == expected).asBool());
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

TEST(ValueTests, ModOperator) {
    expectValue(modValues(10, 3), 1.0);
    expectValue(modValues(7.5, 2), 1.5);
    expectValue(modValues("7.5", "2"), 1.5);
    expectValue(modValues("7.5", 2), 1.5);
    expectValue(modValues(false, true), 0.);
    expectValue(modValues("3", true), 0.);
    expectValue(modValues("-8.0", 5), -3.);
}

TEST(ValueTests, ThrowsOnZeroModDivisor) {
    EXPECT_THROW(modValues(4, "0."), std::runtime_error);
}

TEST(ValueTests, LessThanOperator) {
    expectValue(lessThan("abd", "acd"), true);
    expectValue(lessThan("bdf", "bde"), false);
    expectValue(lessThan(3, 4), true);
    expectValue(lessThan(3, 4.5), true);
    expectValue(lessThan("3.5", 4), true);
}

TEST(ValueTests, GreaterThanOperator) {
    expectValue(greaterThan("abd", "acd"), false);
    expectValue(greaterThan("bdf", "bde"), true);
    expectValue(greaterThan(3, 4), false);
    expectValue(greaterThan(3, 4.5), false);
    expectValue(greaterThan("3.5", 4), false);
}

TEST(ValueTests, LessEqOperator) {
    expectValue(lessEq("abd", "acd"), true);
    expectValue(lessEq("abd", "abd"), true);
    expectValue(lessEq("bdf", "bde"), false);
    expectValue(lessEq(3, 4), true);
    expectValue(lessEq(3, 3.), true);
    expectValue(lessEq(3, 4.5), true);
    expectValue(lessEq("3.5", 4), true);
    expectValue(lessEq("3.5", "3.5"), true);
}

TEST(ValueTests, GreaterEqOperator) {
    expectValue(greaterEq("abd", "acd"), false);
    expectValue(greaterEq("abd", "abd"), true);
    expectValue(greaterEq("bdf", "bde"), true);
    expectValue(greaterEq(3, 4), false);
    expectValue(greaterEq(3, 3.), true);
    expectValue(greaterEq(3, 4.5), false);
    expectValue(greaterEq("3.5", 4), false);
    expectValue(greaterEq("3.5", "3.5"), true);
}

TEST(ValueTests, ComparisonFailsOnVoidBoolArr) {
    EXPECT_THROW((void)(getArrayVal({1, 2}) < Value(2)), std::runtime_error);
    EXPECT_THROW((void)(Value(false) < Value(2)), std::runtime_error);
    EXPECT_THROW((void)(Value() < Value(2)), std::runtime_error);
}

TEST(ValueTests, EqOperator) {
    expectValue(getArrayVal({1, 2, 3}) == getArrayVal({"1.", "2.0", "3."}), true); 
    expectValue(getArrayVal({1, 2, 3}) == getArrayVal({"1.", "2.0", "3.5"}), false); 

    // [[1, 2], [3, 4]]
    Value nestedArr1 = getArrayVal({getArrayVal({1, 2}), getArrayVal({3, 4})});

    // [["1.0", "2."], ["3", "4."]]
    Value nestedArr2 = getArrayVal({getArrayVal({"1.0", "2."}), getArrayVal({"3", "4."})});
    expectValue(nestedArr1 == nestedArr2, true);

    expectValue(equalTo("abc", "def"), false);
    expectValue(equalTo("def", "def"), true);
    expectValue(equalTo(3.5, 3), false);
    expectValue(equalTo(3., 3), true);
    expectValue(equalTo(1, true), true);
    expectValue(equalTo(false, false), true);
}

TEST(ValueTests, NotEqOperator) {
    expectValue(getArrayVal({1, 2, 3}) != getArrayVal({"1.", "2.0", "3."}), false); 
    expectValue(getArrayVal({1, 2, 3}) != getArrayVal({"1.", "2.0", "3.5"}), true); 

    // [[1, 2], [3, 4]]
    Value nestedArr1 = getArrayVal({getArrayVal({1, 2}), getArrayVal({3, 4})});

    // [["1.0", "2."], ["3", "4."]]
    Value nestedArr2 = getArrayVal({getArrayVal({"1.0", "2."}), getArrayVal({"3", "4."})});
    expectValue(nestedArr1 != nestedArr2, false);

    expectValue(notEqTo("abc", "def"), true);
    expectValue(notEqTo("def", "def"), false);
    expectValue(notEqTo(3.5, 3), true);
    expectValue(notEqTo(3., 3), false);
    expectValue(notEqTo(1, true), false);
    expectValue(notEqTo(false, false), false);
}

TEST(ValueTests, Cardinality) {
    expectValue(Value("asdfgh").getCardinality(), 6); 
    expectValue(getArrayVal({1, 2, 3, 4, 5, 6, 7}).getCardinality(), 7); 
}

TEST(ValueTests, CardinalityThrowsOnBadTypes) {
    EXPECT_THROW(Value(3).getCardinality(), std::runtime_error);
    EXPECT_THROW(Value(3.5).getCardinality(), std::runtime_error);
    EXPECT_THROW(Value(true).getCardinality(), std::runtime_error);
}

TEST(ValueTests, NegativeOp) {
    expectValue(Value(3).negateNum(), -3); 
    expectValue(Value(3.5).negateNum(), -3.5); 
    expectValue(Value(0).negateNum(), 0); 
    expectValue(Value(true).negateNum(), -1); 
    expectValue(Value("3.5").negateNum(), -3.5); 
}

TEST(ValueTests, AndOp) {
    expectValue(andOp("asdf", ""), false);
    expectValue(andOp(4, "3.5"), true);
    expectValue(andOp(0, true), false);
}

TEST(ValueTests, OrOp) {
    expectValue(orOp("asdf", ""), true);
    expectValue(orOp(0, ""), false);
    expectValue(orOp(0., true), true);
}

TEST(ValueTests, NotOp) {
    expectValue(notOp("abc"), false);
    expectValue(notOp(0), true);
    expectValue(notOp(0.), true);
    expectValue(notOp(false), true);
}

TEST(ValueTests, logicalThrowsOnArray) {
    EXPECT_THROW((void)(andOp(getArrayVal({1, 2}), 3)), std::runtime_error);
    EXPECT_THROW((void)(orOp(getArrayVal({1, 2}), 3)), std::runtime_error);
    EXPECT_THROW((void)(notOp(getArrayVal({1, 2}))), std::runtime_error);
}

TEST(ValueTests, idxOp) {
    Value arr1 = getArrayVal({1, 2, 3, 4});
    expectValue(arr1[Value(3)], 4);
    expectValue(Value("asdf")[Value(2)], std::string(1, 'd'));
    expectValue(Value(123)[Value(2)], std::string(1, '3'));
    expectValue(Value(34.2)[Value(2)], std::string(1, '.'));
}

TEST(ValueTests, idxOpThrowsOnBadIdx) {
    Value arr1 = getArrayVal({1, 2, 3, 4});

    // Out of range
    EXPECT_THROW((void)Value("asdf")[Value(4)], std::runtime_error);
    EXPECT_THROW((void)arr1[Value(4)], std::runtime_error);

    // Index is not an int
    EXPECT_THROW((void)arr1[Value("2")], std::runtime_error);
    EXPECT_THROW((void)arr1[Value(2.5)], std::runtime_error);
    EXPECT_THROW((void)arr1[Value(true)], std::runtime_error);
    EXPECT_THROW((void)arr1[arr1], std::runtime_error);
}

TEST(ValueTests, concatOp) {
    Value result1 = getArrayVal({1, 2, 3}).concatenate(getArrayVal({4, 5, 6}));
    Value expected1 = getArrayVal({1, 2, 3, 4, 5, 6});
    expectValue(result1 == expected1, true);

    expectValue(concatValues("asdf", "qwer"), std::string("asdfqwer"));
    expectValue(concatValues("asdf", 3), std::string("asdf3"));
    expectValue(concatValues("asdf", 3.5), std::string("asdf3.500000"));
    expectValue(concatValues("asdf", true), std::string("asdftrue"));
}

TEST(ValueTests, concatThrowsOnTypeMismatch) {
    Value arr = getArrayVal({1, 2, 3});
    EXPECT_THROW(arr.concatenate(Value(2)), std::runtime_error);
    EXPECT_THROW(arr.concatenate(Value(2.5)), std::runtime_error);
    EXPECT_THROW(arr.concatenate(Value("asdf")), std::runtime_error);
    EXPECT_THROW(arr.concatenate(Value(true)), std::runtime_error);
}

TEST(ValueTests, splitOp) {
    Value arr = getArrayVal({1, 2, 3, 4, 5}); 
    Value result = arr.split(Value(2));
    Value expected = getArrayVal({3, 4, 5});
    expectValue(expected == result, true);
    expectValue(splitValue("asdf", 2), std::string("df"));
    expectValue(splitValue(3.456, 2), std::string("456000"));
    expectValue(splitValue(3456, 2), std::string("56"));
    expectValue(splitValue(-3456, 2), std::string("456"));
    expectValue(splitValue(true, 2), std::string("ue"));
}

TEST(ValueTests, intersectionOp) {
    Value arr1 = getArrayVal({1, 2, 3, 4});
    Value arr2 = getArrayVal({"2.", "3.", "4.", "5."});
    Value result = arr1.intersection(arr2);
    Value expected = getArrayVal({2, 3, 4});
    expectValue(result == expected, true);

    // [[1, 2], [3, 4], [1, 2]]
    Value nestedArr1 = getArrayVal({getArrayVal({1, 2}), getArrayVal({3, 4}),
            getArrayVal({1, 2})});

    // [["1.0", "2."]]
    Value nestedArr2 = getArrayVal({getArrayVal({"1.0", "2."})});
    Value nestedResult = nestedArr1.intersection(nestedArr2);
    Value nestedExpected = getArrayVal({getArrayVal({1, 2})});
    expectValue(nestedResult == nestedExpected, true); 

    expectValue(intersectValues("asdf", "sdfg"), std::string("sdf"));
    expectValue(intersectValues("asdf", "ghu"), std::string(""));
    expectValue(intersectValues(3456, 432.6), std::string("346"));
    expectValue(intersectValues(true, false), std::string("e"));
}

TEST(ValueTests, appendOp) {
    Value arr1 = getArrayVal({1, 2, 3});    
    Value result = arr1.append(Value(4));
    Value expected = getArrayVal({1, 2, 3, 4});
    expectValue(expected == result, true);
}

TEST(ValueTests, appendThrowsOnNotArray) {
    EXPECT_THROW(Value(4).append(Value(3)), std::runtime_error);
    EXPECT_THROW(Value(4.5).append(Value(3)), std::runtime_error);
    EXPECT_THROW(Value("adsf").append(Value(3)), std::runtime_error);
    EXPECT_THROW(Value(true).append(Value(3)), std::runtime_error);
}

TEST(ValueTests, extractOp) {
    Value arr1 = getArrayVal({1, 2, 3});
    expectValue(arr1.extract(Value(1)), 2);
    
    Value expected = getArrayVal({1, 3});
    expectValue(arr1 == expected, true);
}

TEST(ValueTests, extractOpThrows) {
    // Invalid type
    EXPECT_THROW(Value(1).extract(Value(2)), std::runtime_error);
    EXPECT_THROW(Value(1.5).extract(Value(2)), std::runtime_error);
    EXPECT_THROW(Value("asdf").extract(Value(2)), std::runtime_error);
    EXPECT_THROW(Value(true).extract(Value(2)), std::runtime_error);

    // Index out of bounds
    Value arr1 = getArrayVal({1, 2, 3});
    EXPECT_THROW(arr1.extract(Value(3)), std::runtime_error);
}

TEST(ValueTests, getsDepth) {
    EXPECT_EQ(Value(42).getDepth(), 0);
    EXPECT_EQ(Value(42.5).getDepth(), 0);
    EXPECT_EQ(Value("asdfj").getDepth(), 0);
    EXPECT_EQ(Value(true).getDepth(), 0);

    Value arr1 = Value(std::make_shared<Value::ArrayType>());
    EXPECT_EQ(arr1.getDepth(), 1);

    Value arr2 = getArrayVal({getArrayVal({1, 2}), getArrayVal({3, 4})});
    EXPECT_EQ(arr2.getDepth(), 2);
}

TEST(ValueTests, getDepthThrowsOnInconsistentArrays) {
    Value arr1 = getArrayVal({1, 2});
    Value::ArrayType arrVec = {arr1, Value(2)};
    Value nestedArr = Value(std::make_shared<Value::ArrayType>(arrVec));
    EXPECT_THROW(nestedArr.getDepth(), std::runtime_error);
}

TEST(ValueTests, modifiesString) {
    Value str("asdfg");
    str.modifyString(4, 'z');
    expectValue(str, std::string("asdfz"));
}

TEST(ValueTests, modifyStringThrows) {
    // Index out of bounds
    Value str("asdfg");
    EXPECT_THROW(str.modifyString(5, 'z'), std::runtime_error);

    // Not a string
    EXPECT_THROW(Value(1).modifyString(0, 'a'), std::runtime_error);
    EXPECT_THROW(Value(1.5).modifyString(0, 'a'), std::runtime_error);
    EXPECT_THROW(Value(true).modifyString(0, 'a'), std::runtime_error);
    EXPECT_THROW(getArrayVal({1, 2}).modifyString(0, 'a'), std::runtime_error);
}

