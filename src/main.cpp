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
const flp PI = (3 + 2) * 2
const flp DOT_FIRST = 3 % 1
bool boolean = 4 / 2 

str double_quoted = "Testing escapes: \n \t \r \" \\ " 
str single_quoted = 'Testing escapes: \n \t \r \' \\ '

str b = "asdf"
b[2] = 4


)";

    std::deque<Token> tokenized = lex(source);

    MockLexer lexer{tokenized};
    ErrorHandler errHandler;
    Parser parser{lexer, errHandler};
    Interpreter interpreter;

    Program program = parser.parse();    
    interpreter.visit(program);
    std::cout << interpreter.m_env.get("b") << "\n";

    return 0;
}
