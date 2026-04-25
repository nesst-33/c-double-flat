#include <variant>
#include <string>
#include <iostream>
#include <unordered_map>
#include <string_view>

// TODO: dodać backslash jako kontynuacja linii

namespace Config
{
    inline constexpr int MAX_TOKEN_LEN = 255;
};

enum class TokenType
{
    // Identifiers and r-values
    IDENTIFIER_T,
    INT_VALUE_T,
    STR_VALUE_T,
    FLP_VALUE_T,

    PLUS_T,             // + (both unary and binary)
    MINUS_T,            // - (same here)
    MULT_T,             // *
    DIV_T,              // /
    MOD_T,              // %

    CONCAT_T,           // ~
    CONJUN_T,           // &
    SPLIT_T,            // :
    CARDINALITY_T,      // !
    APPEND_T,           // <<
    EXTRACT_T,          // >>

    GREATER_T,          // >
    LESSER_T,           // <
    EQ_T,               // ==
    NOT_EQ_T,           // !=
    GREATER_EQ_T,       // >=
    LESSER_EQ_T,        // <=

    ASSIGN_T,           // =
    ADD_ASSIGN_T,       // +=
    SUB_ASSIGN_T,       // -=
    MULT_ASSIGN_T,      // *=
    DIV_ASSIGN_T,       // /=
    MOD_ASSIGN_T,       // %=
    CONCAT_ASSIGN_T,    // ~=

    // Keywords (most are self-explanatory)
    IF_T,
    ELSE_T,
    WHILE_T,
    CONST_T,
    AS_T,               // Used for type casting (ex. 3 as str)
    AND_T,
    OR_T,
    NOT_T,
    VOID_T,
    INT_T,
    FLP_T,
    STR_T,
    ARR_T,
    RETURN_T,

    COMMA_T,            // ,
    L_BRACKET_T,        // (
    R_BRACKET_T,        // )
    L_BRACE_T,          // {
    R_BRACE_T,          // }
    L_SQUARE_T,         // [
    R_SQUARE_T,         // ]

    NEWLINE_T,          // \n - End of a statement
    COMMENT_T,
    EOT,                // No more chars in stream 
    UNKNOWN             // Unknown token
};

const std::unordered_map<std::string_view, TokenType> keyword_map = {
    {"if", TokenType::IF_T},
    {"else", TokenType::ELSE_T},
    {"while", TokenType::WHILE_T},
    {"const", TokenType::CONST_T},
    {"as", TokenType::AS_T},
    {"and", TokenType::AND_T},
    {"or", TokenType::OR_T},
    {"not", TokenType::NOT_T},
    {"void", TokenType::VOID_T},
    {"int", TokenType::INT_T},
    {"flp", TokenType::FLP_T},
    {"arr", TokenType::ARR_T},
    {"return", TokenType::RETURN_T}
};

const std::unordered_map<std::string_view, TokenType> operator_map = {
    {"+", TokenType::PLUS_T},
    {"-", TokenType::MINUS_T},
    {"/", TokenType::DIV_T},
    {"*", TokenType::MULT_T},
    {"%", TokenType::MOD_T},
    {"~", TokenType::CONCAT_T},
    {"+=", TokenType::ADD_ASSIGN_T},
    {"-=", TokenType::SUB_ASSIGN_T},
    {"/=", TokenType::DIV_ASSIGN_T},
    {"*=", TokenType::MULT_ASSIGN_T},
    {"%=", TokenType::MOD_ASSIGN_T},
    {"~=", TokenType::CONCAT_ASSIGN_T}
};

struct Position
{
    int offset{}, line{}, column{};
};

struct Token
{
    TokenType type;
    std::variant<std::string, int, double> value; 
    Position position; 
};

bool match(char expected, std::istream& stream)
{
    if (stream.peek() == expected)
    {
        stream.get();
        return true;
    }
    return false;
}

Token handleOperator(std::istream& input, char character)
{
    // Próbowałem się tu wycwanić jako, że każdy z operatorów zawartych
    // w mapie operator_map ma wariant ze znakiem =
    
    std::string op(1, character);
    if (input.peek() == '=')
    {
        input.get();
        op += "=";
    }

    TokenType op_type = operator_map.at(op);
    return Token(op_type, op);
}

Token handleLesser(std::istream& input, char character)
{
    char next_char = input.peek();

    switch (next_char)
    {
        case '<':
            return Token(TokenType::APPEND_T, "<<");
        case '=':
            return Token(TokenType::LESSER_EQ_T, "<=");
        default:
            return Token(TokenType::LESSER_T, "<");
    }
}

Token handleGreater(std::istream& input, char character)
{
}


Token tokenLoop(std::istream& input)
{
    char character{};
    while (true)
    {
        character = input.get();
        if (character == '\n')
            return Token{TokenType::NEWLINE_T, character};

        if (std::isspace(character))
            continue;

        switch (character)
        {
            case '+':
            case '-':
            case '*':
            case '/':
            case '~':
            case '%':
                return handleOperator(input, character);

            case '<':
            case '>':
        }
    }
    
    return Token{};
}


class UniversalReader
{

};

int main()
{
    return 0;
}
