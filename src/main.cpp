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
    if (x > 4) {
        const arr arr int a = [3, 2, 1]
        const arr arr int b = a * 4
    } else {
        int c = 3*4+2
        a = 4
        {

            what += 4[0] * 3
        }
        return i+2*3
    }
    a = 4

    const int testfun(arr array, int cd234) 
    {
        return a[0][try(_, 5)]
    }
    arr test = 5
        )";

    std::deque<Token> tokenized = lex(source);
    

    MockLexer lexer{tokenized};
    Parser parser{lexer};
    ASTPrinter printer;

    Program program = parser.parse();    
    printer.visit(program);
    std::cout << printer.getResult();


    return 0;
}
