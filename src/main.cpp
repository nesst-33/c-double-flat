#include "Lexer.h"
#include "Parser.h"
#include "Interpreter.h"
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
    std::string source = R"(
arr int a = [1, 2, 3, 4]
int b = a >> 2

int mult(int item, int multiplier) {
    return item * multiplier
}
print([1, 2, 3, 4][mult(_, 3)])
)";

    std::deque<Token> tokenized = lex(source);

    MockLexer lexer{tokenized};
    ErrorHandler errHandler;
    Parser parser{lexer, errHandler};
    Interpreter interpreter;

    Program program = parser.parse();    
    interpreter.visit(program);
    errHandler.printErrors();

    return 0;
}
