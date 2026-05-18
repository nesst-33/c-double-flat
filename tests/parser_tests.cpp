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

void checkNumOfErrs(const ErrorHandler& errHandler, int expectedNum) {
    EXPECT_EQ(errHandler.getErrCount(), expectedNum) << errHandler.formatErrors();
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
int k = +-not-+not b
int l = 3 - +-2
int test_parenth = (3 + 2) * 7
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
int k = (+(-(not (-(+(not b))))))
int l = (3 - (+(-2)))
int test_parenth = ((3 + 2) * 7)
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
    checkNumOfErrs(errHandler, 1);
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
    checkNumOfErrs(errHandler, 2);
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
    checkNumOfErrs(errHandler2, 2);
    checkErrorSeverity(errs2[0], Severity::WARNING);
    checkErrorSeverity(errs2[1], Severity::ERROR);
    EXPECT_EQ(output, expected);
}

TEST(NegativeTests, WarnsOfMissingArrayClosingBracket) {
    std::string source = R"(
arr int y = [0, 1, 2
arr flp z = y[0[1]
z[0 = .14
)";
    std::string expected = R"(arr int y = [0, 1, 2]
arr flp z = (y[(0[1])])
(z[0]) = 0.140000
)";

    ErrorHandler errHandler;
    EXPECT_EQ(printParsedProgram(source, errHandler), expected);

    const auto& errs = errHandler.getErrors();
    checkIfOnlySyntaxErrs(errs);
    checkNumOfErrs(errHandler, 3);
    for (const auto& err : errs)
        checkErrorSeverity(err, Severity::WARNING);
}

TEST(NegativeTests, ThrowsOnMissingArrIndex) {
    std::string source = R"(
z[] = true
arr bool y = z[]
)";
    std::string expected = R"()";

    ErrorHandler errHandler;
    EXPECT_EQ(printParsedProgram(source, errHandler), expected);

    const auto& errs = errHandler.getErrors();
    checkIfOnlySyntaxErrs(errs);
    checkNumOfErrs(errHandler, 2);
    for (const auto& err : errs)
        checkErrorSeverity(err, Severity::ERROR);
}

TEST(NegativeTests, ThrowsOnStrayUnaryOperator) {
    std::string source = R"(
int y = x++
bool z = not
    )";
    std::string expected = R"()";

    ErrorHandler errHandler;
    EXPECT_EQ(printParsedProgram(source, errHandler), expected);

    const auto& errs = errHandler.getErrors();
    checkIfOnlySyntaxErrs(errs);
    checkNumOfErrs(errHandler, 2);
    for (const auto& err : errs)
        checkErrorSeverity(err, Severity::ERROR);
}
