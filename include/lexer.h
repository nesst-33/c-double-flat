#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include <iostream>
#include <unordered_map>
#include <string_view>
#include <stdexcept>

class LexerException : public std::runtime_error
{
public:
    LexerException(const std::string& msg, Position pos)
        : std::runtime_error("Lexer Error: " + msg), m_pos(pos) {}
    Position getPosition() const { return m_pos; }
private:
    Position m_pos;
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
    static constexpr int MAX_ID_LEN = 255;

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


#endif
