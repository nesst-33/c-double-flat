#ifndef _PARSER_H
#define _PARSER_H

#include <vector>
#include <initializer_list>
#include <optional>
#include <memory>
#include "Lexer.h"

class ErrorHandler {
public:
    void handleError() {}
};

class Visitor;

class Node {
};


class Program : public Node {
    std::vector<std::unique_ptr<Node>> m_statements{};
public:
    Program(std::vector<std::unique_ptr<Node>> statements) 
        : m_statements(std::move(statements)) {}
};

class Statement : public Program {};

class IfStmt : public Statement {};
class WhileStmt : public Statement {};
class Scope : public Statement {};
class VarOrFuncDecl : public Statement {};
class VoidFuncDecl : public Statement {};
class RetStmt : public Statement {};
class IdArrFuncCall : public Statement {};


// class NodeVisitor : public Visitor {
// 
// };

class Parser {
public:
    Parser(ILexer& lexer) : lexer(lexer)
    {
        nextToken();
    }

    std::unique_ptr<Program> parseProgram() {
        std::vector<std::unique_ptr<Statement>> statements{};

        while (auto st = parseStatement())
            statements.push_back(std::move(st));

        return std::make_unique<Program>(std::move(statements));
    }
private:
    ILexer& lexer;
    Token currToken{};
    ErrorHandler errHandler();

    void nextToken() 
    { 
        currToken = lexer.getToken(); 
        while (currToken.type == TokenType::COMMENT_T)
            currToken = lexer.getToken();
    }

    void advance() { return; }
    
    bool check(TokenType type) { return false; }

    bool match(std::initializer_list<TokenType> types) 
    {
        for (TokenType type : types) 
        {
            if (check(type))
            {
                advance();
                return true;
            }
        }
        return false;
    } 
    std::unique_ptr<Statement> parseStatement() { return {}; }
};

#endif
