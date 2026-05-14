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


class ILexer 
{
public:
    virtual Token getToken() = 0;
    virtual Position getPosition() const = 0;
    virtual ~ILexer() = default;
};

class MockLexer : public ILexer {
public:
    MockLexer(std::deque<Token> tokenList) : tokenQueue(std::move(tokenList)) {}
    Token getToken() override {
        Token token = tokenQueue.front();
        tokenQueue.pop();
        return token;
    }
    Position getPosition() const override {
        return tokenQueue.front().position;
    }
private:
    std::queue<Token> tokenQueue;
};

class Lexer : public ILexer
{
public:
    Lexer(std::istream& input) : m_input(input) {}
    Position getPosition() const override { return currentPos; }
    Token getToken() override;

private:
    std::istream& m_input;
    std::string m_buffer{};
    Position currentPos{1, 0, -1};
    // TODO: startPos
    bool lineBreak{false};
    static constexpr int MAX_ID_LEN = 255;

    // arbitrary maximum string literal/single line comment length (ex. to avoid DOS)
    static constexpr int MAX_STR_LEN = 1024;

    char getChar();
    bool match(char character);
    Token handleOperator(char character, Position startPos);
    Token handleComment(Position startPos);
    Token buildNumber(char character, Position startPos);
    Token buildIdOrKeyword(char character, Position startPos);
    Token buildString(char quoteType, Position startPos);
    
    char handleEscapeSeq(Position startPos);
    double buildDecimal(int digits);
    void clearWhitespace();

    static const std::unordered_map<std::string_view, TokenType> keyword_map; 
    static const std::unordered_map<std::string_view, TokenType> operator_map;

};


#endif
