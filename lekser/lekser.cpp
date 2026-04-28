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

struct Position
{
    int line{};
    int column{};
    size_t offset{}; // NOTE: this is a logical character offset; NOT A BYTE OFFSET!
};

class LexerException : public std::runtime_error
{
public:
    LexerException(const std::string& msg, Position pos)
        : std::runtime_error("Lexer Error: " + msg), m_pos(pos) {}

    Position getPosition() const { return m_pos; }

private:
    Position m_pos;
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
    Position getPosition() const { return currentPos; }
    Token getToken();

private:
    std::istream& m_input;
    std::string m_buffer{};
    Position currentPos{1, 0, -1};
    bool lineBreak{false};

    char getChar();
    bool match(char character);
    Token handleOperator(char character, Position startPos);
    Token handleComment(Position startPos);
    Token buildNumber(char character, Position startPos);
    Token buildIdOrKeyword(char character, Position startPos);
    Token buildString(char quoteType, Position startPos);
    
    char handleEscapeSeq(Position startPos);
    double buildDecimal();
    void clearWhitespace();

    static const std::unordered_map<std::string_view, TokenType> keyword_map; 
    static const std::unordered_map<std::string_view, TokenType> operator_map;

};

const std::unordered_map<std::string_view, TokenType> Lexer::keyword_map = {
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

const std::unordered_map<std::string_view, TokenType> Lexer::operator_map = {
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

    m_buffer.clear(); 

    if (m_buffer.capacity() < 64)
        m_buffer.reserve(64);

    m_buffer.push_back('#');

    while (m_input.peek() != '\n' && m_input.peek() != EOF)
        m_buffer.push_back(getChar());

    return Token(TokenType::COMMENT_T, m_buffer, startPos);
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

Token Lexer::buildIdOrKeyword(char character, Position startPos) 
{
    char identifier_chars[Config::MAX_ID_LEN] = {character};
    int i {1};
    
    // Najpierw ciąg znaków alfanumerycznych traktowany jest jako identyfikator
    while (std::isalnum(m_input.peek()) || m_input.peek() == '_')
    {
        if (i < Config::MAX_ID_LEN)
        {
            identifier_chars[i] = getChar();
            i++;
        }
        else
            throw LexerException("Identifier too long (max 255 characters)", startPos);
    }
    
    // Potem próbujemy go dopasować do jakiegoś słowa klucza
    std::string identifier {identifier_chars};
    if (auto keyword_iterator = keyword_map.find(identifier); 
            keyword_iterator != keyword_map.end())
    {
        return Token(keyword_iterator->second, keyword_iterator->first);
    }
    
    return Token(TokenType::IDENTIFIER_T, identifier, startPos);
}

Token Lexer::buildString(char quoteType, Position startPos)
{
    // NOTE: returned token doesn't store enclosing quotes (don't need them)

    // We'll use the same buffer as the comment handler to (maybe) reuse existing memory
    // Also the string value length is also technically unlimited
    m_buffer.clear(); 

    while (true)
    {
        char ch = getChar();

        if (m_input.eof())
            throw LexerException("Unterminated string literal", startPos);

        if (ch == quoteType)
            break;
        
        if (ch == '\n')
            throw LexerException("Newline in string literal", startPos);

        if (ch == '\\')
            m_buffer.push_back(handleEscapeSeq(startPos));
        else
            m_buffer.push_back(ch);
    }
    return Token(TokenType::STR_VALUE_T, m_buffer, startPos);
}

char Lexer::handleEscapeSeq(Position startPos)
{
    char next = getChar();
    if (m_input.eof())
        throw LexerException("Unterminated escape sequence at EOF", startPos);
    
    switch (next)
    {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case '\\': return '\\';
        case '"': return '"';
        case '\'': return '\'';
        default:
            return next;
    }
}

void Lexer::clearWhitespace()
{
    while (true)
    {
        char ch = m_input.peek();

        if (std::isspace(ch) && ch != '\n')
        {
            getChar();
            continue;
        }

        // Backslash signals line continuation
        if (ch == '\\')
        {
            Position startPos = currentPos;
            getChar();

            // There can only be a newline after a backslash
            if (getChar() == '\n')
                continue;
            else
                throw LexerException("Stray backslash (no newline after backslash)", startPos);
        }

        break;
    }
}

Token Lexer::getToken()
{
    char character = getChar();
    Position startPos = currentPos;

    if (m_input.eof())
        return Token(TokenType::EOT);
    if (character == '\n')
        return Token(TokenType::NEWLINE_T, '\n', startPos);

    clearWhitespace();

    switch (character)
    {
        case '+':
        case '-':
        case '*':
        case '/':
        case '~':
        case '%':
            return handleOperator(character, startPos);

        case '<':
            if (match('<')) return Token(TokenType::APPEND_T, "<<", startPos);
            if (match('=')) return Token(TokenType::LESSER_EQ_T, "<=", startPos);
            return Token(TokenType::LESSER_T, "<", startPos);
            
        case '>':
            if (match('>')) return Token(TokenType::EXTRACT_T, ">>", startPos);
            if (match('=')) return Token(TokenType::GREATER_EQ_T, ">=", startPos);
            return Token(TokenType::GREATER_T, ">", startPos);

        case '=':
            if (match('=')) return Token(TokenType::EQ_T, "==", startPos);
            return Token(TokenType::ASSIGN_T, "=", startPos);

        case '!':
            if (match('=')) return Token(TokenType::NOT_EQ_T, "!=", startPos);
            return Token(TokenType::CARDINALITY_T, "!", startPos);
        
        case ':': return Token(TokenType::SPLIT_T, ":", startPos);
        case '&': return Token(TokenType::CONJUN_T, "&", startPos);
        case ',': return Token(TokenType::COMMA_T, ",", startPos);
        case '(': return Token(TokenType::L_BRACKET_T, "(", startPos);
        case ')': return Token(TokenType::R_BRACKET_T, ")", startPos);
        case '{': return Token(TokenType::L_BRACE_T, "{", startPos);
        case '}': return Token(TokenType::R_BRACE_T, "}", startPos);
        case '[': return Token(TokenType::L_SQUARE_T, "[", startPos);
        case ']': return Token(TokenType::R_SQUARE_T, "]", startPos);

        case '\'':
        case '"': 
            return Lexer::buildString(character, startPos);

        case '#': return Lexer::handleComment(startPos);

        // For building flp values that start with . (like .314)
        case '.':
            if (std::isdigit(m_input.peek()))
                return Token(TokenType::FLP_VALUE_T, buildDecimal(), startPos);

        default:
            if (std::isdigit(character)) 
                return buildNumber(character, startPos);
            if (std::isalpha(character))
                return buildIdOrKeyword(character, startPos);
            return Token(TokenType::UNKNOWN, std::string(1, character), startPos);
    }
}

