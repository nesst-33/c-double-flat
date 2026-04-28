#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include "Lexer.h"

// HELPERS (needed since we should use std::get to verify the held type in std::variant)
void expectStringToken(Lexer& lexer, TokenType expectedType, const std::string& expectedVal)
{
    Token t = lexer.getToken();
    EXPECT_EQ(t.type, expectedType) << "Expected type " << static_cast<int>(expectedType) 
                                    << " but got " << static_cast<int>(t.type);
    if (t.type != TokenType::EOT && t.type != TokenType::UNKNOWN)
        EXPECT_EQ(std::get<std::string>(t.value), expectedVal);
}

void expectIntToken(Lexer& lexer, int expectedVal)
{
    Token t = lexer.getToken();
    EXPECT_EQ(t.type, TokenType::INT_VALUE_T);
    EXPECT_EQ(std::get<int>(t.value), expectedVal);
}

void expectDoubleToken(Lexer& lexer, double expectedVal)
{
    Token t = lexer.getToken();
    EXPECT_EQ(t.type, TokenType::FLP_VALUE_T);
    EXPECT_DOUBLE_EQ(std::get<double>(t.value), expectedVal);
}

// Basic parsing verification
TEST(BasicLexerTests, ParsesSingleCharOperators)
{
    std::istringstream input("+ - * / % ~ < > = ! : & , ( ) { } [ ]");
    Lexer lexer(input);

    expectStringToken(lexer, TokenType::PLUS_T, "+");
    expectStringToken(lexer, TokenType::MINUS_T, "-");
    expectStringToken(lexer, TokenType::MULT_T, "*");
    expectStringToken(lexer, TokenType::DIV_T, "/");
    expectStringToken(lexer, TokenType::MOD_T, "%");
    expectStringToken(lexer, TokenType::CONCAT_T, "~");
    expectStringToken(lexer, TokenType::LESSER_T, "<");
    expectStringToken(lexer, TokenType::GREATER_T, ">");
    expectStringToken(lexer, TokenType::ASSIGN_T, "=");
    expectStringToken(lexer, TokenType::CARDINALITY_T, "!");
    expectStringToken(lexer, TokenType::SPLIT_T, ":");
    expectStringToken(lexer, TokenType::CONJUN_T, "&");
    expectStringToken(lexer, TokenType::COMMA_T, ",");
    expectStringToken(lexer, TokenType::L_BRACKET_T, "(");
    expectStringToken(lexer, TokenType::R_BRACKET_T, ")");
    expectStringToken(lexer, TokenType::L_BRACE_T, "{");
    expectStringToken(lexer, TokenType::R_BRACE_T, "}");
    expectStringToken(lexer, TokenType::L_SQUARE_T, "[");
    expectStringToken(lexer, TokenType::R_SQUARE_T, "]");
    EXPECT_EQ(lexer.getToken().type, TokenType::EOT);
}

TEST(BasicLexerTests, ParsesDoubleCharOperators)
{
    std::istringstream input("+= -= *= /= %= ~= << >> <= >= == !=");
    Lexer lexer(input);

    expectStringToken(lexer, TokenType::ADD_ASSIGN_T, "+=");
    expectStringToken(lexer, TokenType::SUB_ASSIGN_T, "-=");
    expectStringToken(lexer, TokenType::MULT_ASSIGN_T, "*=");
    expectStringToken(lexer, TokenType::DIV_ASSIGN_T, "/=");
    expectStringToken(lexer, TokenType::MOD_ASSIGN_T, "%=");
    expectStringToken(lexer, TokenType::CONCAT_ASSIGN_T, "~=");
    expectStringToken(lexer, TokenType::APPEND_T, "<<");
    expectStringToken(lexer, TokenType::EXTRACT_T, ">>");
    expectStringToken(lexer, TokenType::LESSER_EQ_T, "<=");
    expectStringToken(lexer, TokenType::GREATER_EQ_T, ">=");
    expectStringToken(lexer, TokenType::EQ_T, "==");
    expectStringToken(lexer, TokenType::NOT_EQ_T, "!=");
    EXPECT_EQ(lexer.getToken().type, TokenType::EOT);
}

// TEST(MethodTests, FindsKeywords)

TEST(BasicLexerTests, ParsesKeywords) {
    std::istringstream input("if else while const as and or not void int flp str arr return");
    Lexer lexer(input);

    expectStringToken(lexer, TokenType::IF_T, "if");
    expectStringToken(lexer, TokenType::ELSE_T, "else");
    expectStringToken(lexer, TokenType::WHILE_T, "while");
    expectStringToken(lexer, TokenType::CONST_T, "const");
    expectStringToken(lexer, TokenType::AS_T, "as");
    expectStringToken(lexer, TokenType::AND_T, "and");
    expectStringToken(lexer, TokenType::OR_T, "or");
    expectStringToken(lexer, TokenType::NOT_T, "not");
    expectStringToken(lexer, TokenType::VOID_T, "void");
    expectStringToken(lexer, TokenType::INT_T, "int");
    expectStringToken(lexer, TokenType::FLP_T, "flp");
    expectStringToken(lexer, TokenType::STR_T, "str");
    expectStringToken(lexer, TokenType::ARR_T, "arr");
    expectStringToken(lexer, TokenType::RETURN_T, "return");
}
// TEST(BasicLexerTests, CalculatesPosition)
// {
//     std::istringstream input("12 3.14\nelse abc");
//     Lexer lexer(input);
// 
//     Token token = lexer.getToken();
//     EXPECT_EQ(token.position, (Position{1, 1, 0}));
//     
//     token = lexer.getToken();
//     EXPECT_EQ(token.position, (Position{1, 4, 3})) << token.position.column << token.position.line << token.position.offset;
// }




