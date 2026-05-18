#include <gtest/gtest.h>
#include <deque>
#include <memory>
#include <sstream>
#include <string_view>
#include "Parser.h"
#include "ASTPrinter.h"
#include "Lexer.h"


// Helper function that takes in a source string and outputs a deque of Tokens
// used by the MockLexer
std::deque<Token> lex(std::string_view source) {
    std::istringstream stream((std::string(source)));
    Lexer l(stream);
    std::deque<Token> tokens;
    Token t;
    do {
        t = l.getToken();
        tokens.push_back(t);
    } while (t.type != TokenType::EOT);
    tokens.push_back(l.getToken());
    return tokens;
}

// IMPORTANT NOTE:
// To avoid the tedious task of writing out tokens manually, in my unit tests
// I first get a deque of Tokens from lex (which uses a temporary Lexer object)
// and then pass this deque into my MockLexer.
//
// After parsing, I use the ASTPrinter class, which reconstructs the code from
// the AST, while also parenthesizing all expressions (to express the order of operations).
//

std::string printParsedProgram(std::string_view source, ErrorHandler& errHandler) {
    std::deque<Token> tokenized = lex(source);
    MockLexer lexer(tokenized);
    Parser parser(lexer, errHandler);
    ASTPrinter printer;

    Program parsed = parser.parse();
    printer.visit(parsed);
    return printer.getResult();
}

template<typename T>
bool checkErrorClass(const std::unique_ptr<LangError>& error) {
    return dynamic_cast<T*>(error.get()) != nullptr;
}

// Checks if the given vector contains only syntax errors
void checkIfOnlySyntaxErrs(const std::vector<std::unique_ptr<LangError>>& errors) {
    for (const auto& err : errors)
        ASSERT_TRUE(checkErrorClass<SyntaxError>(err));
}

void checkErrorSeverity(const std::unique_ptr<LangError>& error, Severity severity) {
    EXPECT_EQ(error->getSeverity(), severity) << error->what();
}


TEST(SingleStatementTests, OrderOfOperations) {
    std::string source = R"(
int x = 2 + 3 * 7
int y = 3 / 7 - 2 + 3
int z = 10 / 2 * 3 % 4
int a = a ~ b & c << d
int b = x as str as int
int c = 1 + 2 << 3 - 4
int d = a & b * c
int e = -x!
int f = not flags[0]
int g = x as int!
int h = matrix[0][1] as flp
bool i = x + 1 > y and z == 0
bool j = true and false or false and false
bool final_boss = not matrix[0][1]! as flp * 5 + 10 << 2 > value and ready
)"; 

    std::string expected = R"(int x = (2 + (3 * 7))
int y = (((3 / 7) - 2) + 3)
int z = (((10 / 2) * 3) % 4)
int a = (((a ~ b) & c) << d)
int b = ((x as str) as int)
int c = ((1 + 2) << (3 - 4))
int d = (a & (b * c))
int e = (-(x!))
int f = (not (flags[0]))
int g = ((x as int)!)
int h = (((matrix[0])[1]) as flp)
bool i = (((x + 1) > y) and (z == 0))
bool j = ((true and false) or (false and false))
bool final_boss = ((((((not ((((matrix[0])[1])!) as flp)) * 5) + 10) << 2) > value) and ready)
)";

    ErrorHandler errHandler;
    std::string output = printParsedProgram(source, errHandler);
    EXPECT_EQ(errHandler.getErrCount(), 0);
    ASSERT_EQ(output, expected) << output;
}

TEST(NegativeTests, WarnsOfMissingNewline) {
    std::string source = R"(int x = 4)";
    std::string expected = R"(int x = 4
)";

    ErrorHandler errHandler;
    std::string output = printParsedProgram(source, errHandler);
    const auto& errors = errHandler.getErrors();

    checkIfOnlySyntaxErrs(errors);
    EXPECT_EQ(errors.size(), 1);
    checkErrorSeverity(errors[0], Severity::WARNING);
    EXPECT_EQ(output, expected);
}

TEST(NegativeTests, ThrowsOnChainedRelationalOps) {
    std::string source = R"(
bool z = a == b == c
)";
    
    std::string expected = R"(bool z = (a == b)
)";

    ErrorHandler errHandler;
    std::string output = printParsedProgram(source, errHandler);
    const auto& errors = errHandler.getErrors();

    checkIfOnlySyntaxErrs(errors);
    EXPECT_EQ(errHandler.getErrCount(), 2);
    checkErrorSeverity(errors[0], Severity::WARNING);
    checkErrorSeverity(errors[1], Severity::ERROR);
    EXPECT_EQ(output, expected);

    source = R"(
bool z = a < b > c
    )";

    expected = R"(bool z = (a < b)
)";

    ErrorHandler errHandler2;
    output = printParsedProgram(source, errHandler2);
    const auto& errs2 = errHandler2.getErrors();

    checkIfOnlySyntaxErrs(errs2);
    EXPECT_EQ(errHandler2.getErrCount(), 2);
    checkErrorSeverity(errs2[0], Severity::WARNING);
    checkErrorSeverity(errs2[1], Severity::ERROR);
    EXPECT_EQ(output, expected);
}







