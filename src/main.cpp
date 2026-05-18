#include "Lexer.h"
#include "Parser.h"
#include "ASTPrinter.h"
#include <deque>
#include <sstream>

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
    std::deque<Token> tokenStream = {
        // Token(TokenType::IDENTIFIER_T, {}, "test"),
        Token(TokenType::ASSIGN_T),
        Token(TokenType::INT_VALUE_T, {}, 3),
        Token(TokenType::PLUS_T, {}),
        Token(TokenType::INT_VALUE_T, {}, 2),
        Token(TokenType::MULT_T, {}),
        Token(TokenType::INT_VALUE_T, {}, 4),
        Token(TokenType::NEWLINE_T),
        Token(TokenType::IDENTIFIER_T, {}, "test"),
        Token(TokenType::ASSIGN_T),
        Token(TokenType::INT_VALUE_T, {}, 3),
        Token(TokenType::PLUS_T, {}),
        Token(TokenType::INT_VALUE_T, {}, 2),
        Token(TokenType::MULT_T, {}),
        Token(TokenType::INT_VALUE_T, {}, 4),
        Token(TokenType::NEWLINE_T),
        Token(TokenType::EOT)
    };

    std::string source = R"(
    bool z = not
        )";

    std::deque<Token> tokenized = lex(source);

    MockLexer lexer{tokenized};
    ErrorHandler errHandler;
    Parser parser{lexer, errHandler};
    ASTPrinter printer;

    Program program = parser.parse();    
    printer.visit(program);
    std::cout << printer.getResult();
    errHandler.printErrors();

    return 0;
}
