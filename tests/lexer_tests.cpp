#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <fstream>
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

TEST(BasicLexerTests, ParsesKeywords) 
{
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

TEST(BasicLexerTests, ParsesIdentifiers) 
{
    std::istringstream input("myVar _hiddenVar value_123 IF RETURN 1bad_name");
    Lexer lexer(input);

    expectStringToken(lexer, TokenType::IDENTIFIER_T, "myVar");
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "_hiddenVar");
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "value_123");
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "IF");
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "RETURN");
    EXPECT_EQ(lexer.getToken().type, TokenType::INT_VALUE_T); // identifier can't start with a number
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "bad_name");
}

TEST(BasicLexerTests, ParsesIntegers)
{
    std::istringstream input("0 42 99999");
    Lexer lexer(input);

    expectIntToken(lexer, 0);
    expectIntToken(lexer, 42);
    expectIntToken(lexer, 99999);
}

TEST(BasicLexerTests, ParsesFlp)
{
    std::istringstream input("3.14 0.0001 42. .31");
    Lexer lexer(input);

    expectDoubleToken(lexer, 3.14);
    expectDoubleToken(lexer, 0.0001);
    expectDoubleToken(lexer, 42.0);
    expectDoubleToken(lexer, 0.31);
}

TEST(BasicLexerTests, ParsesStandardStrings)
{
    std::istringstream input("\"Hello World\" 'Single Quotes'");
    Lexer lexer(input);

    expectStringToken(lexer, TokenType::STR_VALUE_T, "Hello World");
    expectStringToken(lexer, TokenType::STR_VALUE_T, "Single Quotes");
}

TEST(BasicLexerTests, ParsesEmptyStrings)
{
    std::istringstream input ("\"\" ''");
    Lexer lexer(input);
    expectStringToken(lexer, TokenType::STR_VALUE_T, "");
    expectStringToken(lexer, TokenType::STR_VALUE_T, "");
}

TEST(BasicLexerTests, ParsesStringEscapeSequences)
{
    // String in code: "Line1\nLine2\t\"quote\""
    std::istringstream input("\"Line1\\nLine2\\t\\\"quote\\\"\"");
    Lexer lexer(input);

    expectStringToken(lexer, TokenType::STR_VALUE_T, "Line1\nLine2\t\"quote\"");
}

TEST(BasicLexerTests, SkipsWhitespaceButNotNewline)
{
    std::istringstream input("    \t x \n    \t y\n");
    Lexer lexer(input);

    expectStringToken(lexer, TokenType::IDENTIFIER_T, "x");
    expectStringToken(lexer, TokenType::NEWLINE_T, "\n");
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "y");
    expectStringToken(lexer, TokenType::NEWLINE_T, "\n");
    EXPECT_EQ(lexer.getToken().type, TokenType::EOT);
}

TEST(BasicLexerTests, HandlesComments)
{
    std::istringstream input("x = 5 # Ciekawy komentarz 3 * 5\ny = 10");
    Lexer lexer(input);

    expectStringToken(lexer, TokenType::IDENTIFIER_T, "x");
    expectStringToken(lexer, TokenType::ASSIGN_T, "=");
    expectIntToken(lexer, 5);
    
    expectStringToken(lexer, TokenType::COMMENT_T, "# Ciekawy komentarz 3 * 5");
    expectStringToken(lexer, TokenType::NEWLINE_T, "\n");
    
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "y");
    expectStringToken(lexer, TokenType::ASSIGN_T, "=");
    expectIntToken(lexer, 10);
}

TEST(BasicLexerTests, TracksPositions)
{
    std::istringstream input("x = 5\n  y = 10");
                // offsets:   012345-678
    Lexer lexer(input);

    Token t1 = lexer.getToken(); // 'x'
    EXPECT_EQ(t1.position.line, 1);
    EXPECT_EQ(t1.position.column, 1);
    EXPECT_EQ(t1.position.offset, 0);

    lexer.getToken(); // '='
    lexer.getToken(); // '5'
    lexer.getToken(); // '\n'

    Token t5 = lexer.getToken(); // 'y'
    EXPECT_EQ(t5.position.line, 2);
    EXPECT_EQ(t5.position.column, 3); 
    EXPECT_EQ(t5.position.offset, 8);
}

TEST(BasicLexerTests, HandlesLineContinuation)
{
    std::istringstream input("x=5 \\\n      \\\n + 2");
    Lexer lexer(input);

    expectStringToken(lexer, TokenType::IDENTIFIER_T, "x");
    expectStringToken(lexer, TokenType::ASSIGN_T, "=");
    expectIntToken(lexer, 5);
    
    expectStringToken(lexer, TokenType::PLUS_T, "+");
    expectIntToken(lexer, 2);
    EXPECT_EQ(lexer.getToken().type, TokenType::EOT);
}

TEST(BasicLexerTests, HandlesUnknownChars)
{
    std::istringstream input("@ $ ^");
    Lexer lexer(input);

    expectStringToken(lexer, TokenType::UNKNOWN, "@");
    expectStringToken(lexer, TokenType::UNKNOWN, "$");
    expectStringToken(lexer, TokenType::UNKNOWN, "^");
}


// Throw tests (sad paths)
TEST(LexerThrowTests, ThrowsStrayBacklash)
{
    std::istringstream input("x = 5 \\ + 2");
    Lexer lexer(input);

    for (int i{}; i < 3; i++) lexer.getToken();

    EXPECT_THROW(lexer.getToken(), LexerException);
}

TEST(LexerThrowTests, ThrowsBacklashBeforeEOF)
{
    std::istringstream input("abcd\nab\\");
    Lexer lexer(input);

    for (int i{}; i < 3; i++) lexer.getToken();

    EXPECT_THROW(lexer.getToken(), LexerException);
}

TEST(LexerThrowTests, ThrowsLongIdentifier)
{
    std::string longId(255, 'a');
    std::istringstream input(longId);
    Lexer lexer(input);

    EXPECT_NO_THROW(lexer.getToken());

    std::string longerId(256, 'a');
    std::istringstream input2(longerId);
    Lexer lexer2(input2);

    EXPECT_THROW(lexer2.getToken(), LexerException);
}

TEST(LexerThrowTests, ThrowsUnterminatedLiteral)
{
    std::istringstream input("'there is no end to unit test hell");
    Lexer lexer(input);
    
    EXPECT_THROW(lexer.getToken(), LexerException);
}

TEST(LexerThrowTests, ThrowsNewlineInString)
{
    std::istringstream input("'do not break\nit up'");
    Lexer lexer(input);
    EXPECT_THROW(lexer.getToken(), LexerException);
}

TEST(LexerThrowTests, ThrowsEOFEscapeSeq)
{
    std::istringstream input("'A melodia się urywa niby Hejnał Mariacki\\");
    Lexer lexer(input);
    EXPECT_THROW(lexer.getToken(), LexerException);
}

// Integration tests
TEST(LexerIntegrationTests, ProcessesSampleSourceFile)
{
    std::ifstream testFile("../../tests/test.txt", std::ios::in | std::ios::binary);
    ASSERT_TRUE(testFile.is_open());
    
    Lexer lexer(testFile);

    // --- LINE 1 ---
    expectStringToken(lexer, TokenType::COMMENT_T, "# TEST INTEGRACYJNY NR 1");
    expectStringToken(lexer, TokenType::NEWLINE_T, "\n");

    // --- LINE 2 ---
    expectStringToken(lexer, TokenType::CONST_T, "const");
    expectStringToken(lexer, TokenType::INT_T, "int");
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "MAX");
    expectStringToken(lexer, TokenType::ASSIGN_T, "=");
    expectIntToken(lexer, 100);
    expectStringToken(lexer, TokenType::NEWLINE_T, "\n");

    // --- LINE 3 ---
    expectStringToken(lexer, TokenType::STR_T, "str");
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "path");
    expectStringToken(lexer, TokenType::ASSIGN_T, "=");
    expectStringToken(lexer, TokenType::STR_VALUE_T, "C:\\temp\\file.txt"); // Check escapes!
    expectStringToken(lexer, TokenType::NEWLINE_T, "\n");

    // --- LINE 4 & 5 (Line Continuation Magic) ---
    expectStringToken(lexer, TokenType::FLP_T, "flp");
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "value");
    expectStringToken(lexer, TokenType::ASSIGN_T, "=");
    expectDoubleToken(lexer, 0.5); // The leading decimal
    
    // The Lexer should swallow the `\ \n` and jump straight to the `+` on line 5
    Token plusToken = lexer.getToken();
    EXPECT_EQ(plusToken.type, TokenType::PLUS_T);
    EXPECT_EQ(plusToken.position.line, 5); // Verify line counter kept working!
    
    expectDoubleToken(lexer, 42.0);
    expectStringToken(lexer, TokenType::NEWLINE_T, "\n");

    // --- LINE 6 ---
    expectStringToken(lexer, TokenType::IF_T, "if");
    expectStringToken(lexer, TokenType::L_BRACKET_T, "(");
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "value");
    expectStringToken(lexer, TokenType::GREATER_EQ_T, ">=");
    expectIntToken(lexer, 10);
    expectStringToken(lexer, TokenType::AND_T, "and");
    expectStringToken(lexer, TokenType::NOT_T, "not");
    expectIntToken(lexer, 0);
    expectStringToken(lexer, TokenType::OR_T, "or");
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "value");
    expectStringToken(lexer, TokenType::LESSER_T, "<");
    expectIntToken(lexer, 5);
    expectStringToken(lexer, TokenType::R_BRACKET_T, ")");
    expectStringToken(lexer, TokenType::L_BRACE_T, "{");
    expectStringToken(lexer, TokenType::NEWLINE_T, "\n");

    // --- LINE 7 ---
    expectStringToken(lexer, TokenType::ARR_T, "arr");
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "data");
    expectStringToken(lexer, TokenType::ASSIGN_T, "=");
    expectStringToken(lexer, TokenType::L_SQUARE_T, "[");
    expectIntToken(lexer, 1);
    expectStringToken(lexer, TokenType::COMMA_T, ",");
    expectIntToken(lexer, 2);
    expectStringToken(lexer, TokenType::COMMA_T, ",");
    expectIntToken(lexer, 3);
    expectStringToken(lexer, TokenType::R_SQUARE_T, "]");
    expectStringToken(lexer, TokenType::NEWLINE_T, "\n");

    // --- LINE 8 ---
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "data");
    expectStringToken(lexer, TokenType::APPEND_T, "<<");
    expectIntToken(lexer, 4);
    expectStringToken(lexer, TokenType::NEWLINE_T, "\n");

    // --- LINE 9 ---
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "data");
    expectStringToken(lexer, TokenType::L_SQUARE_T, "[");
    expectIntToken(lexer, 0);
    expectStringToken(lexer, TokenType::R_SQUARE_T, "]");
    expectStringToken(lexer, TokenType::MOD_ASSIGN_T, "%=");
    expectIntToken(lexer, 2);
    expectStringToken(lexer, TokenType::NEWLINE_T, "\n");

    // --- LINE 10 ---
    expectStringToken(lexer, TokenType::RETURN_T, "return");
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "value");
    expectStringToken(lexer, TokenType::AS_T, "as");
    expectStringToken(lexer, TokenType::INT_T, "int");
    expectStringToken(lexer, TokenType::NEWLINE_T, "\n");

    // --- LINE 11 ---
    expectStringToken(lexer, TokenType::R_BRACE_T, "}");
    expectStringToken(lexer, TokenType::ELSE_T, "else");
    expectStringToken(lexer, TokenType::L_BRACE_T, "{");
    expectStringToken(lexer, TokenType::NEWLINE_T, "\n");

    // --- LINE 12 ---
    expectStringToken(lexer, TokenType::VOID_T, "void");
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "func");
    expectStringToken(lexer, TokenType::L_BRACKET_T, "(");
    expectStringToken(lexer, TokenType::R_BRACKET_T, ")");
    expectStringToken(lexer, TokenType::L_BRACE_T, "{");
    expectStringToken(lexer, TokenType::WHILE_T, "while");
    expectStringToken(lexer, TokenType::L_BRACKET_T, "(");
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "value");
    expectStringToken(lexer, TokenType::NOT_EQ_T, "!=");
    expectIntToken(lexer, 0);
    expectStringToken(lexer, TokenType::R_BRACKET_T, ")");
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "value");
    expectStringToken(lexer, TokenType::SUB_ASSIGN_T, "-=");
    expectIntToken(lexer, 1);
    expectStringToken(lexer, TokenType::R_BRACE_T, "}");
    expectStringToken(lexer, TokenType::NEWLINE_T, "\n");

    // --- LINE 13 ---
    expectStringToken(lexer, TokenType::R_BRACE_T, "}");
    expectStringToken(lexer, TokenType::NEWLINE_T, "\n");

    // --- LINE 14 (Remaining Operators) ---
    expectStringToken(lexer, TokenType::CONCAT_T, "~");
    expectStringToken(lexer, TokenType::CONCAT_ASSIGN_T, "~=");
    expectStringToken(lexer, TokenType::DIV_T, "/");
    expectStringToken(lexer, TokenType::DIV_ASSIGN_T, "/=");
    expectStringToken(lexer, TokenType::MULT_T, "*");
    expectStringToken(lexer, TokenType::MULT_ASSIGN_T, "*=");
    expectStringToken(lexer, TokenType::EXTRACT_T, ">>");
    expectStringToken(lexer, TokenType::EQ_T, "==");
    expectStringToken(lexer, TokenType::CARDINALITY_T, "!");
    expectStringToken(lexer, TokenType::NEWLINE_T, "\n");
    
    // --- EOF ---
    EXPECT_EQ(lexer.getToken().type, TokenType::EOT);
}
