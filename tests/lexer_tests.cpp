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
    ErrorHandler errHandler;
    Lexer lexer(input, errHandler); // TODO: różne leksery na każdy operator + EOF

    EXPECT_EQ(lexer.getToken().type, TokenType::PLUS_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::MINUS_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::MULT_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::DIV_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::MOD_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::CONCAT_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::LESSER_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::GREATER_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::ASSIGN_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::CARDINALITY_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::SPLIT_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::CONJUN_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::COMMA_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::L_BRACKET_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::R_BRACKET_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::L_BRACE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::R_BRACE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::L_SQUARE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::R_SQUARE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::EOT);
}

TEST(BasicLexerTests, ParsesDoubleCharOperators)
{
    std::istringstream input("+= -= *= /= %= ~= << >> <= >= == !="); 
    ErrorHandler errHandler;
    Lexer lexer(input, errHandler);

    EXPECT_EQ(lexer.getToken().type, TokenType::ADD_ASSIGN_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::SUB_ASSIGN_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::MULT_ASSIGN_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::DIV_ASSIGN_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::MOD_ASSIGN_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::CONCAT_ASSIGN_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::APPEND_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::EXTRACT_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::LESSER_EQ_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::GREATER_EQ_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::EQ_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::NOT_EQ_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::EOT);
}

TEST(BasicLexerTests, ParsesKeywords) 
{
    std::istringstream input("if else while const as and or not void int flp str arr return true false bool");
    ErrorHandler errHandler;
    Lexer lexer(input, errHandler);

    EXPECT_EQ(lexer.getToken().type, TokenType::IF_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::ELSE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::WHILE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::CONST_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::AS_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::AND_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::OR_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::NOT_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::VOID_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::INT_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::FLP_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::STR_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::ARR_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::RETURN_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::TRUE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::FALSE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::BOOL_T);
}

TEST(BasicLexerTests, ParsesIdentifiers) 
{
    std::istringstream input("myVar _hiddenVar value_123 IF RETURN 1bad_name");
    ErrorHandler errHandler; Lexer lexer(input, errHandler);

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
    ErrorHandler errHandler; Lexer lexer(input, errHandler);

    expectIntToken(lexer, 0);
    expectIntToken(lexer, 42);
    expectIntToken(lexer, 99999);
}

TEST(BasicLexerTests, ParsesFlp)
{
    // TODO: add tests for big flp
    std::istringstream input("3.14 0.0001 42. .31");
    ErrorHandler errHandler; Lexer lexer(input, errHandler);

    expectDoubleToken(lexer, 3.14);
    expectDoubleToken(lexer, 0.0001);
    expectDoubleToken(lexer, 42.0);
    expectDoubleToken(lexer, 0.31);
}

TEST(BasicLexerTests, ParsesStandardStrings)
{
    std::istringstream input("\"Hello World\" 'Single Quotes'");
    ErrorHandler errHandler; Lexer lexer(input, errHandler);

    expectStringToken(lexer, TokenType::STR_VALUE_T, "Hello World");
    expectStringToken(lexer, TokenType::STR_VALUE_T, "Single Quotes");
}

TEST(BasicLexerTests, ParsesEmptyStrings)
{
    std::istringstream input ("\"\" ''");
    ErrorHandler errHandler; Lexer lexer(input, errHandler);
    expectStringToken(lexer, TokenType::STR_VALUE_T, "");
    expectStringToken(lexer, TokenType::STR_VALUE_T, "");
}

TEST(BasicLexerTests, ParsesStringEscapeSequences)
{
    // String in code: "Line1\nLine2\t\"quote\""
    std::istringstream input("\"Line1\\nLine2\\t\\\"quote\\\"\"");
    ErrorHandler errHandler; Lexer lexer(input, errHandler);

    expectStringToken(lexer, TokenType::STR_VALUE_T, "Line1\nLine2\t\"quote\"");
}

TEST(BasicLexerTests, SkipsWhitespaceButNotNewline)
{
    std::istringstream input("    \t x \n    \t y\n");
    ErrorHandler errHandler; Lexer lexer(input, errHandler);

    expectStringToken(lexer, TokenType::IDENTIFIER_T, "x");
    EXPECT_EQ(lexer.getToken().type, TokenType::NEWLINE_T);
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "y");
    EXPECT_EQ(lexer.getToken().type, TokenType::NEWLINE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::EOT);
}

TEST(BasicLexerTests, HandlesComments)
{
    // TODO: Sprawdzić czy działa brak nowej linii po komentarzu
    std::istringstream input("x = 5 # Ciekawy komentarz 3 * 5\ny = 10");
    ErrorHandler errHandler; Lexer lexer(input, errHandler);

    expectStringToken(lexer, TokenType::IDENTIFIER_T, "x");
    EXPECT_EQ(lexer.getToken().type, TokenType::ASSIGN_T);
    expectIntToken(lexer, 5);
    
    expectStringToken(lexer, TokenType::COMMENT_T, "# Ciekawy komentarz 3 * 5");
    EXPECT_EQ(lexer.getToken().type, TokenType::NEWLINE_T);
    
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "y");
    EXPECT_EQ(lexer.getToken().type, TokenType::ASSIGN_T);
    expectIntToken(lexer, 10);
}

TEST(BasicLexerTests, TracksPositions)
{
    std::istringstream input("x = 5\n  y = 10");
                // offsets:   012345-678
    ErrorHandler errHandler; Lexer lexer(input, errHandler);

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
    ErrorHandler errHandler; Lexer lexer(input, errHandler);

    expectStringToken(lexer, TokenType::IDENTIFIER_T, "x");
    EXPECT_EQ(lexer.getToken().type, TokenType::ASSIGN_T);
    expectIntToken(lexer, 5);
    
    EXPECT_EQ(lexer.getToken().type, TokenType::PLUS_T);
    expectIntToken(lexer, 2);
    EXPECT_EQ(lexer.getToken().type, TokenType::EOT);
}

TEST(BasicLexerTests, HandlesUnknownChars)
{
    std::istringstream input("@ $ ^");
    ErrorHandler errHandler; Lexer lexer(input, errHandler);

    expectStringToken(lexer, TokenType::UNKNOWN, "@");
    expectStringToken(lexer, TokenType::UNKNOWN, "$");
    expectStringToken(lexer, TokenType::UNKNOWN, "^");
}


// Throw tests (sad paths)
TEST(LexerThrowTests, ThrowsStrayBacklash)
{
    std::istringstream input("x = 5 \\ + 2");
    ErrorHandler errHandler;
    Lexer lexer(input, errHandler);

    for (int i{}; i < 3; i++) lexer.getToken();

    EXPECT_THROW(lexer.getToken(), LexerException);
}

TEST(LexerThrowTests, ThrowsBacklashBeforeEOF)
{
    std::istringstream input("abcd\nab\\");
    ErrorHandler errHandler;
    Lexer lexer(input, errHandler);

    for (int i{}; i < 3; i++) lexer.getToken();

    EXPECT_THROW(lexer.getToken(), LexerException);
}

TEST(LexerThrowTests, ThrowsLongIdentifier)
{
    std::string longId(255, 'a');
    std::istringstream input(longId);
    ErrorHandler errHandler;
    Lexer lexer(input, errHandler);

    EXPECT_NO_THROW(lexer.getToken());

    std::string longerId(256, 'a');
    std::istringstream input2(longerId);
    ErrorHandler errHandler2;
    Lexer lexer2(input2, errHandler2);

    EXPECT_THROW(lexer2.getToken(), LexerException);
}

TEST(LexerThrowTests, ThrowsUnterminatedLiteral)
{
    std::istringstream input("'there is no end to unit test hell");
    ErrorHandler errHandler;
    Lexer lexer(input, errHandler);
    
    EXPECT_THROW(lexer.getToken(), LexerException);
}

TEST(LexerThrowTests, ThrowsNewlineInString)
{
    std::istringstream input("'do not break\nit up'");
    ErrorHandler errHandler;
    Lexer lexer(input, errHandler);
    EXPECT_THROW(lexer.getToken(), LexerException);
}

TEST(LexerThrowTests, ThrowsEOFEscapeSeq)
{
    std::istringstream input("'A melodia się urywa niby Hejnał Mariacki\\");
    ErrorHandler errHandler;
    Lexer lexer(input, errHandler);
    EXPECT_THROW(lexer.getToken(), LexerException);
}

// Integration tests
TEST(LexerIntegrationTests, ProcessesSampleSourceFile)
{
    std::ifstream testFile("../../tests/test.txt", std::ios::in | std::ios::binary);
    ASSERT_TRUE(testFile.is_open());
    
    ErrorHandler errHandler;
    Lexer lexer(testFile, errHandler);

    // --- LINE 1 ---
    expectStringToken(lexer, TokenType::COMMENT_T, "# TEST INTEGRACYJNY NR 1");
    EXPECT_EQ(lexer.getToken().type, TokenType::NEWLINE_T);

    // --- LINE 2 ---
    EXPECT_EQ(lexer.getToken().type, TokenType::CONST_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::INT_T);
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "MAX");
    EXPECT_EQ(lexer.getToken().type, TokenType::ASSIGN_T);
    expectIntToken(lexer, 100);
    EXPECT_EQ(lexer.getToken().type, TokenType::NEWLINE_T);

    // --- LINE 3 ---
    EXPECT_EQ(lexer.getToken().type, TokenType::STR_T);
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "path");
    EXPECT_EQ(lexer.getToken().type, TokenType::ASSIGN_T);
    expectStringToken(lexer, TokenType::STR_VALUE_T, "C:\\temp\\file.txt"); 
    EXPECT_EQ(lexer.getToken().type, TokenType::NEWLINE_T);

    // --- LINE 4 & 5 ---
    EXPECT_EQ(lexer.getToken().type, TokenType::FLP_T);
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "value");
    EXPECT_EQ(lexer.getToken().type, TokenType::ASSIGN_T);
    expectDoubleToken(lexer, 0.5); 
    
    // The Lexer should swallow the `\ \n` and jump straight to the `+` on line 5
    Token plusToken = lexer.getToken();
    EXPECT_EQ(plusToken.type, TokenType::PLUS_T);
    EXPECT_EQ(plusToken.position.line, 5); 
    
    expectDoubleToken(lexer, 42.0);
    EXPECT_EQ(lexer.getToken().type, TokenType::NEWLINE_T);

    // --- LINE 6 ---
    EXPECT_EQ(lexer.getToken().type, TokenType::IF_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::L_BRACKET_T);
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "value");
    EXPECT_EQ(lexer.getToken().type, TokenType::GREATER_EQ_T);
    expectIntToken(lexer, 10);
    EXPECT_EQ(lexer.getToken().type, TokenType::AND_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::NOT_T);
    expectIntToken(lexer, 0);
    EXPECT_EQ(lexer.getToken().type, TokenType::OR_T);
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "value");
    EXPECT_EQ(lexer.getToken().type, TokenType::LESSER_T);
    expectIntToken(lexer, 5);
    EXPECT_EQ(lexer.getToken().type, TokenType::R_BRACKET_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::L_BRACE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::NEWLINE_T);

    // --- LINE 7 ---
    EXPECT_EQ(lexer.getToken().type, TokenType::ARR_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::IDENTIFIER_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::ASSIGN_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::L_SQUARE_T);
    expectIntToken(lexer, 1);
    EXPECT_EQ(lexer.getToken().type, TokenType::COMMA_T);
    expectIntToken(lexer, 2);
    EXPECT_EQ(lexer.getToken().type, TokenType::COMMA_T);
    expectIntToken(lexer, 3);
    EXPECT_EQ(lexer.getToken().type, TokenType::R_SQUARE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::NEWLINE_T);

    // --- LINE 8 ---
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "data");
    EXPECT_EQ(lexer.getToken().type, TokenType::APPEND_T);
    expectIntToken(lexer, 4);
    EXPECT_EQ(lexer.getToken().type, TokenType::NEWLINE_T);

    // --- LINE 9 ---
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "data");
    EXPECT_EQ(lexer.getToken().type, TokenType::L_SQUARE_T);
    expectIntToken(lexer, 0);
    EXPECT_EQ(lexer.getToken().type, TokenType::R_SQUARE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::MOD_ASSIGN_T);
    expectIntToken(lexer, 2);
    EXPECT_EQ(lexer.getToken().type, TokenType::NEWLINE_T);

    // --- LINE 10 ---
    EXPECT_EQ(lexer.getToken().type, TokenType::RETURN_T);
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "value");
    EXPECT_EQ(lexer.getToken().type, TokenType::AS_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::INT_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::NEWLINE_T);

    // --- LINE 11 ---
    EXPECT_EQ(lexer.getToken().type, TokenType::R_BRACE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::ELSE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::L_BRACE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::NEWLINE_T);

    // --- LINE 12 ---
    EXPECT_EQ(lexer.getToken().type, TokenType::VOID_T);
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "func");
    EXPECT_EQ(lexer.getToken().type, TokenType::L_BRACKET_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::R_BRACKET_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::L_BRACE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::WHILE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::L_BRACKET_T);
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "value");
    EXPECT_EQ(lexer.getToken().type, TokenType::NOT_EQ_T);
    expectIntToken(lexer, 0);
    EXPECT_EQ(lexer.getToken().type, TokenType::R_BRACKET_T);
    expectStringToken(lexer, TokenType::IDENTIFIER_T, "value");
    EXPECT_EQ(lexer.getToken().type, TokenType::SUB_ASSIGN_T);
    expectIntToken(lexer, 1);
    EXPECT_EQ(lexer.getToken().type, TokenType::R_BRACE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::NEWLINE_T);

    // --- LINE 13 ---
    EXPECT_EQ(lexer.getToken().type, TokenType::R_BRACE_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::NEWLINE_T);

    // --- LINE 14 ---
    EXPECT_EQ(lexer.getToken().type, TokenType::CONCAT_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::CONCAT_ASSIGN_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::DIV_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::DIV_ASSIGN_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::MULT_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::MULT_ASSIGN_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::EXTRACT_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::EQ_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::CARDINALITY_T);
    EXPECT_EQ(lexer.getToken().type, TokenType::NEWLINE_T);
    
    // --- EOF ---
    EXPECT_EQ(lexer.getToken().type, TokenType::EOT);
}
