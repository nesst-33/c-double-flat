#include "Lexer.h"
#include "Parser.h"
#include "Interpreter.h"
#include <deque>
#include <fstream>
#include <istream>
#include <sstream>
#include "ErrorPrinter.h"
#include <iostream>

std::deque<Token> lex(std::string_view source, ErrorHandler& errHandler) {
    std::istringstream stream((std::string(source)));
    Lexer l(stream, errHandler);
    std::deque<Token> tokens;
    Token t;
    do {
        t = l.getToken();
        tokens.push_back(t);
    } while (t.type != TokenType::EOT);
    tokens.push_back(l.getToken());
    return tokens;
}

void run(std::istream& stream, const std::string& sourceName) {
    try {
        ErrorHandler errHandler;
        Lexer lexer{stream, errHandler};
        Parser parser{lexer, errHandler};

        auto programAST = parser.parse();
        Interpreter interpreter{errHandler};

        interpreter.interpret(programAST);

        ErrorPrinter printer{errHandler.getErrors()};
        printer.printErrors(std::cerr, sourceName);
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: cflat [filepath]\n";
        return 64;
    }
    else if (argc == 2) {
        std::string filepath = argv[1];
        std::ifstream file(filepath);

        if (!file.is_open()) {
            std::cerr << "Error: Could not open file '" << filepath << "'\n";
            return 74;
        }

        run(file, filepath);
    }
    else {
        run(std::cin, "<stdin>");
    }

    return 0;
}
//     std::string source = R"(
//
// int fib(int n) {
//     if (n <= 0) {
//         return 0
//     }
//     if (n == 1) {
//         return 1
//     }
//     return fib(n-1) + fib(n-2)
// }
//
// # print(fib(10))
// print(10/0)
// )";
//
//     std::deque<Token> tokenized = lex(source);
//
//     MockLexer lexer{tokenized};
//     ErrorHandler errHandler;
//     Parser parser{lexer, errHandler};
//     Interpreter interpreter{errHandler};
//
//     Program program = parser.parse();    
//
//     try {
//         interpreter.visit(program);
//     } catch (const LangError& e) {}
//
//     ErrorPrinter printer{errHandler.getErrors()};
//     printer.printErrors(std::cerr, "stdin");
//
//
//
//     return 0;
// }
