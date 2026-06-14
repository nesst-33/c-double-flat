#include "Lexer.h"
#include "Parser.h"
#include "Interpreter.h"
#include <deque>
#include <sstream>
#include "ErrorPrinter.h"
#include <iostream>

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

int main()
{
    std::string source = R"(

int fib(int n) {
    if (n <= 0) {
        return 0
    }
    if (n == 1) {
        return 1
    }
    return fib(n-1) + fib(n-2)
}

# print(fib(10))
print(10/0)
)";

    std::deque<Token> tokenized = lex(source);

    MockLexer lexer{tokenized};
    ErrorHandler errHandler;
    Parser parser{lexer, errHandler};
    Interpreter interpreter{errHandler};

    Program program = parser.parse();    

    try {
        interpreter.visit(program);
    } catch (const LangError& e) {}

    ErrorPrinter printer{errHandler.getErrors()};
    printer.printErrors(std::cerr, "stdin");

    

    return 0;
}
