#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include "Lexer.h"

// HELPERS (needed since we should use std::get to verify the held type in std::variant)
void expectStringToken(Lexer& lexer, TokenType expectedType, const std::string& expectedVal)
{
    Token t = lexer.getToken();
    EXPECT_EQ(t.type, expectedType);
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




