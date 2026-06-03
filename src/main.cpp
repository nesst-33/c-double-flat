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
const flp PI = 3.14
const flp DOT_FIRST = .14
const flp DOT_LAST = 3.

str double_quoted = "Testing escapes: \n \t \r \" \\ "
str single_quoted = 'Testing escapes: \n \t \r \' \\ '

const arr arr int GLOBAL_MATRIX = [[1, 2], [3, 4]]

void test_control_flow(int limit, bool is_active) {
    int i = 0
    
    while (i < limit) {
        if ((i % 2 == 0) and is_active) {
            {
                int scoped_var = 1
                scoped_var += i
            }
        } else {
            bool flag = false
            flag ~= true
        }
        
        i += 1
    }
}

arr arr flp complex_operations(arr flp input_arr, flp scalar) {
    arr flp local_arr = input_arr
    
    local_arr[0] = 10.
    local_arr[1] -= 2.5
    local_arr[2] *= scalar
    local_arr[3] /= 2.
    local_arr[4] %= 1.5
    
    arr flp merged = local_arr ~ [ 1.1, 2.2 ] : [ 3.3 ] << 1 >> 2 & [ 4.4 ]
    
    return [ merged, [ scalar, .5 ] ]
}

int main() {
    int base_val = 10
    
    test_control_flow(base_val, true)
    
    arr flp floats = [ 1., 2., 3., 4., 5. ]
    arr arr flp matrix_result = complex_operations(floats, 2.5)
    
    flp extracted = matrix_result[0][(1 + 1)]
    
    int precedence_monster = not 5! * 2 + base_val ~ 20 < 30 == true and false or true
    
    int cast_test = 3.14 as int! + base_val as flp as int
    
    return precedence_monster
}
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
