#include <variant>
#include <string>
#include <iostream>
#include <unordered_map>
#include <string_view>
#include <cctype>
#include <stdexcept>

namespace Config
{
    // Max identifier length
    inline constexpr int MAX_ID_LEN = 255;
};

class LexerException : public std::runtime_error
{
// TODO: Dodać zwracanie pozycji do błędu
public:
    LexerException(const std::string& msg)
        : std::runtime_error("Lexer Error: " + msg) {}
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


struct Position
{
    int line{};
    int column{};
    size_t offset{}; // NOTE: this is a logical character offset; NOT A BYTE OFFSET!
};

struct Token
{
    TokenType type;
    std::variant<std::string_view, int, double> value; 
    Position position; 
};

class Lexer
{
public:
    Lexer(std::istream& input) : m_input(input) {}

    Position getPosition() const
    {
        return currentPos;
    }

    Token getToken();


private:
    std::istream& m_input;
    std::string comment_buffer{};
    Position currentPos{1, 0, 0};
    bool lineBreak{false};

    static inline const std::unordered_map<std::string_view, TokenType> keyword_map = {
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

    static inline const std::unordered_map<std::string_view, TokenType> operator_map = {
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

    char getChar();
    bool match(char character);
    Token handleOperator(char character, Position startPos);
    Token handleComment(Position startPos);
    Token buildNumber(char character, Position startPos);
    Token buildIdOrKeyword(char character, Position startPos);

    double buildDecimal();

};

char Lexer::getChar()
{
    char ch = m_input.get();

    // I could just use .tellg() here but imo it's better to avoid a system call if I can
    currentPos.offset++; 

    if (lineBreak)
    {
        lineBreak = false;
        currentPos.line++;
        currentPos.column = 1;
    }
    else
    {
        currentPos.column++;
    }

    if (ch == '\n')
    {
        lineBreak = true;
    }

    return ch;
}

bool Lexer::match(char character)
{
    if (m_input.peek() == character)
    {
        getChar();
        return true;
    }
    return false;
}

Token Lexer::handleOperator(char character, Position startPos)
{
    // Próbowałem się tu wycwanić jako, że każdy z operatorów zawartych
    // w mapie operator_map ma wariant ze znakiem =
    
    std::string op(1, character);
    if (m_input.peek() == '=')
    {
        getChar();
        op += "=";
    }

    TokenType op_type = operator_map.at(op);
    return Token(op_type, op, startPos);
}


Token Lexer::handleComment(Position startPos)
{
    // The length of a comment is unrestricted, so using a C-style array
    // for appending new characters would be inefficient.
    // The best way I found to build out a comment into a token is to allocate
    // a 64 char string to avoid the smaller reallocations and then add new chars to it.

    // Whatever the implementation of std::string is, I think C-style reallocation
    // at best would match it, but it would probably be worse

    comment_buffer.clear(); 

    if (comment_buffer.capacity() < 64)
        comment_buffer.reserve(64);

    comment_buffer.push_back('#');

    while (m_input.peek() != '\n' && m_input.peek() != EOF)
        comment_buffer.push_back(getChar());

    return Token(TokenType::COMMENT_T, comment_buffer, startPos);
}

Token Lexer::buildNumber(char character, Position startPos) 
{
    int total {character - '0'};

    while (std::isdigit(m_input.peek()))
    {
        total *= 10;
        total += getChar() - '0';
    }
    
    if (m_input.peek() == '.')
    {
        getChar();
        double flp_total = total + buildDecimal();
        return Token(TokenType::FLP_VALUE_T, flp_total);
    }
    return Token {TokenType::INT_VALUE_T, total, startPos};
}


double Lexer::buildDecimal()
{
    double decimalExpansion {0.0};
    double divisor {10.0};

    while (std::isdigit(m_input.peek()))
    {
        decimalExpansion += (getChar() - '0') / divisor;
        divisor *= 10;
    }

    return decimalExpansion;
}

Token buildIdOrKeyword(std::istream& input, char character) 
{
    char identifier_chars[Config::MAX_ID_LEN] = {character};
    int i {1};
    
    // Najpierw ciąg znaków alfanumerycznych traktowany jest jako identyfikator
    while (std::isalnum(input.peek()) || input.peek() == '_')
    {
        if (i < Config::MAX_ID_LEN)
        {
            identifier_chars[i] = input.get();
            i++;
        }
        else
            throw LexerException("Identifier too long (max 255 characters)");
    }
    
    // Potem próbujemy go dopasować do jakiegoś słowa klucza
    std::string identifier {identifier_chars};
    if (auto keyword_iterator = keyword_map.find(identifier); 
            keyword_iterator != keyword_map.end())
    {
        return Token(keyword_iterator->second, keyword_iterator->first);
    }
    
    return Token(TokenType::IDENTIFIER_T, identifier);
}

Token Lexer::getToken()
{
    char character{};

    // I have to skip whitespace but still capture newlines
    do
    {
        character = getChar();
        if (character == '\n')
            return Token{TokenType::NEWLINE_T, character, currentPos};
    } while (std::isspace(character));

    Position startPos = currentPos;

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
            if (match(input, '<')) return Token(TokenType::APPEND_T, "<<");
            if (match(input, '=')) return Token(TokenType::LESSER_EQ_T, "<=");
            return Token(TokenType::LESSER_T, "<");
            
        case '>':
            if (match(input, '>')) return Token(TokenType::EXTRACT_T, ">>");
            if (match(input, '=')) return Token(TokenType::GREATER_EQ_T, ">=");
            return Token(TokenType::GREATER_T);

        case '=':
            if (match(input, '=')) return Token(TokenType::EQ_T, "==");
            return Token(TokenType::ASSIGN_T, "=");

        case '!':
            if (match(input, '=')) return Token(TokenType::NOT_EQ_T, "!=");
            return Token(TokenType::CARDINALITY_T, "!");
        
        case ':': return Token(TokenType::SPLIT_T, ":");
        case '&': return Token(TokenType::CONJUN_T, "&");
        case ',': return Token(TokenType::COMMA_T, ",");
        case '(': return Token(TokenType::L_BRACKET_T, "(");
        case ')': return Token(TokenType::R_BRACKET_T, ")");
        case '{': return Token(TokenType::L_BRACE_T, "{");
        case '}': return Token(TokenType::R_BRACE_T, "}");
        case '[': return Token(TokenType::L_SQUARE_T, "[");
        case ']': return Token(TokenType::R_SQUARE_T, "]");

        case '#': return Lexer::handleComment();

        // Backslash can be used as line continuation
        case '\\':
                while (std::isspace(input.peek()))
                {
                    input.get();
                    continue;
                }
                continue;

        // For building flp values that start with . (like .314)
        case '.':
            if (std::isdigit(input.peek()))
                return Token(TokenType::FLP_VALUE_T, buildDecimal(input));
        
        case EOF:
            return Token(TokenType::EOT, "");

        default:
            if (std::isdigit(character)) 
                return buildNumber(input, character);
            if (std::isalpha(character))
                return buildIdOrKeyword(input, character);
            return Token(TokenType::UNKNOWN, std::string(1, character));
}

Token tokenLoop(std::istream& input)
{
    char character{};
    Lexer lexer(input);
    while (true)
    {
        character = lexer.getChar();
        }
    }
}

int main()
{
    return 0;
}
