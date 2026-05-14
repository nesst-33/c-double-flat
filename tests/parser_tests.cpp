#include <gtest/gtest.h>
#include "Parser.h"

class MockLexer : public ILexer {
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




