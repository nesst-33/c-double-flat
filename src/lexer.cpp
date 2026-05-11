#include "Lexer.h"
#include <cctype>
#include <limits>

// TODO: change to array
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
    {"str", TokenType::STR_T},
    {"arr", TokenType::ARR_T},
    {"return", TokenType::RETURN_T},
    {"true", TokenType::TRUE_T},
    {"false", TokenType::FALSE_T}
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
        lineBreak = false; // TODO: nextChar() i getCurrentChar()
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
    return Token(op_type, startPos);
}


Token Lexer::handleComment(Position startPos)
{
    // The length of a comment is unrestricted, so using a C-style array
    // for appending new characters would be inefficient.
    // The best way I found to build out a comment into a token is to allocate
    // a 64 char string to avoid the smaller reallocations and then add new chars to it.

    // Whatever the implementation of std::string is, I think C-style reallocation
    // at best would match it, but it would probably be worse

    m_buffer.clear(); // TODO: stworzyć lokalnie zmienną i zrobić move

    if (m_buffer.capacity() < 64)
        m_buffer.reserve(64);

    m_buffer.push_back('#');

    while (m_input.peek() != '\n' && m_input.peek() != EOF)
    {
        m_buffer.push_back(getChar());
        if (m_buffer.length() > MAX_STR_LEN)
            throw LexerException("Comment exceeds the max number of characters", startPos);
    }

    return Token(TokenType::COMMENT_T, startPos, m_buffer);
}

Token Lexer::buildNumber(char character, Position startPos) 
{
    if (!std::isdigit(character))
        throw LexerException("Invalid digit in integer literal", startPos);

    int total {character - '0'};
    
    // Needed to calculate the number of digits to avoid needless reading of flp decimal expansion digits
    int digits{1}; 

    while (std::isdigit(m_input.peek()))
    {
        if (total > std::numeric_limits<int>::max() / 10)
            throw LexerException("Integer literal exceeds INT_MAX", startPos);
        total *= 10;

        int nextNum = getChar() - '0';
        if (total > std::numeric_limits<int>::max() - nextNum)
            throw LexerException("Integer literal exceeds INT_MAX", startPos);

        total += nextNum;
        digits++;
    }
    
    if (m_input.peek() == '.')
    {
        getChar();
        double flp_total = total + buildDecimal(digits);
        return Token(TokenType::FLP_VALUE_T, startPos, flp_total);
    }
    return Token(TokenType::INT_VALUE_T, startPos, total);
}


double Lexer::buildDecimal(int digits = 0)
{
    double decimalExpansion {0.0};
    double divisor {10.0};

    while (std::isdigit(m_input.peek()))
    {
        char nextDigit = getChar();

        // We don't need to read any more digits after we hit the limit of accurate
        // representation. I've added one extra digit for rounding purposes
        
        // TODO: add tests for big flp's
        if (digits <= std::numeric_limits<double>::max_digits10 + 1)
        {
            decimalExpansion += (nextDigit - '0') / divisor;
            divisor *= 10;
        }

        digits++;
    }

    return decimalExpansion;
}

Token Lexer::buildIdOrKeyword(char character, Position startPos) 
{
    if (!std::isalnum(character) && character != '_')
        throw LexerException("Not a valid identifier or keyword character", startPos);

    char identifier_chars[MAX_ID_LEN] = {character};
    int i {1};
    
    // Najpierw ciąg znaków alfanumerycznych traktowany jest jako identyfikator
    while (std::isalnum(m_input.peek()) || m_input.peek() == '_')
    {
        if (i < MAX_ID_LEN)
        {
            identifier_chars[i] = getChar();
            i++;
        }
        else
        {
            while (std::isalnum(m_input.peek()) || m_input.peek() == '_') 
            { getChar(); }
            throw LexerException("Identifier too long (max 255 characters)", startPos);
        }
    }
    std::string identifier(identifier_chars, i);
    if (auto keyword_iterator = keyword_map.find(identifier); 
            keyword_iterator != keyword_map.end())
    {
        return Token(keyword_iterator->second, startPos);
    }
    
    return Token(TokenType::IDENTIFIER_T, startPos, identifier);
}

Token Lexer::buildString(char quoteType, Position startPos)
{
    // NOTE: returned token doesn't store enclosing quotes (don't need them)

    // We'll use the same buffer as the comment handler to (maybe) reuse existing memory
    // Also the string value length is also technically unlimited

    m_buffer.clear();  // TODO: stworzyć lokalnie zmienną i zrobić move

    char ch = getChar();
    while (ch != quoteType)
    {
        if (m_input.eof())
            throw LexerException("Unfinished string literal", startPos);

        if (ch == '\n')
            throw LexerException("Newline in string literal", startPos);

        if (ch == '\\')
            m_buffer.push_back(handleEscapeSeq(startPos));
        else
            m_buffer.push_back(ch);

        if (m_buffer.length() > MAX_STR_LEN)
            throw LexerException("String literal length exceeds the max number of character", startPos);
        ch = getChar();
    }
    return Token(TokenType::STR_VALUE_T, startPos, m_buffer);
}

char Lexer::handleEscapeSeq(Position startPos)
{
    char next = getChar();
    if (m_input.eof())
        throw LexerException("Incomplete escape sequence at EOF", startPos);
    
    switch (next)
    {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case '\\': return '\\';
        case '"': return '"';
        case '\'': return '\'';
        default:
            throw LexerException("Unknown escape sequence", startPos);
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

            // For Windows CRLF format
            if (m_input.peek() == '\r')
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
    // TODO: break out into smaller functions
    clearWhitespace();

    char character = getChar();
    Position startPos = currentPos;

    if (m_input.eof())
        return Token(TokenType::EOT);
    if (character == '\n')
        return Token(TokenType::NEWLINE_T, startPos);

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
            if (match('<')) return Token(TokenType::APPEND_T, startPos);
            if (match('=')) return Token(TokenType::LESSER_EQ_T, startPos);
            return Token(TokenType::LESSER_T, startPos);
            
        case '>':
            if (match('>')) return Token(TokenType::EXTRACT_T, startPos);
            if (match('=')) return Token(TokenType::GREATER_EQ_T, startPos);
            return Token(TokenType::GREATER_T, startPos);

        case '=':
            if (match('=')) return Token(TokenType::EQ_T, startPos);
            return Token(TokenType::ASSIGN_T, startPos);

        case '!':
            if (match('=')) return Token(TokenType::NOT_EQ_T, startPos);
            return Token(TokenType::CARDINALITY_T, startPos);
        
        case ':': return Token(TokenType::SPLIT_T, startPos);
        case '&': return Token(TokenType::CONJUN_T, startPos);
        case ',': return Token(TokenType::COMMA_T, startPos);
        case '(': return Token(TokenType::L_BRACKET_T, startPos);
        case ')': return Token(TokenType::R_BRACKET_T, startPos);
        case '{': return Token(TokenType::L_BRACE_T, startPos);
        case '}': return Token(TokenType::R_BRACE_T, startPos);
        case '[': return Token(TokenType::L_SQUARE_T, startPos);
        case ']': return Token(TokenType::R_SQUARE_T, startPos);

        case '\'':
        case '"': 
            return Lexer::buildString(character, startPos);

        case '#': return Lexer::handleComment(startPos);

        // For building flp values that start with . (like .314)
        case '.':
            if (std::isdigit(m_input.peek()))
                return Token(TokenType::FLP_VALUE_T, startPos, buildDecimal());

        default:
            if (std::isdigit(character)) 
                return buildNumber(character, startPos);
            if (std::isalpha(character) || character == '_')
                return buildIdOrKeyword(character, startPos);
            return Token(TokenType::UNKNOWN, startPos, std::string(1, character));
    }
}

