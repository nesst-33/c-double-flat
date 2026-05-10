#ifndef _PARSER_H
#define _PARSER_H

#include <vector>
#include <initializer_list>
#include <memory>
#include "Lexer.h"
#include "Token.h"

class ErrorHandler {
public:
    void handleError() {}
};

class SyntaxError : public std::runtime_error
{
public:
    SyntaxError(const std::string& msg, Position pos)
        : std::runtime_error("Syntax Error: " + msg), m_pos(pos) {}
private:
    Position m_pos;
};

class Node {
};

class Additive : public Node {

};

class ArrayOps : public Node {
private:
};

class Relational : public Node {
public:
    Relational(std::unique_ptr<ArrayOps> leftArrOp,
            std::unique_ptr<ArrayOps> rightArrOp)
        : m_leftArrOp(std::move(leftArrOp))
        , m_rightArrOp(std::move(rightArrOp)) {}
private:
    std::unique_ptr<ArrayOps> m_leftArrOp, m_rightArrOp;
};

class Equality : public Node {
public:
    Equality(std::unique_ptr<Relational> lRelational, std::unique_ptr<Relational> rRelational) 
    : m_leftRelational(std::move(lRelational)) 
    , m_rightRelational(std::move(rRelational)) {}
private:
    std::unique_ptr<Relational> m_leftRelational, m_rightRelational;
};

class LogicalAnd : public Node {
public:
    LogicalAnd(std::vector<std::unique_ptr<Equality>> equalities)
        : m_equalities(std::move(equalities)) {}
private:
    std::vector<std::unique_ptr<Equality>> m_equalities{};
};

class Expression : public Node {
public:
    Expression(std::vector<std::unique_ptr<LogicalAnd>> logicalAnds)
        : m_ands(std::move(logicalAnds)) {}
private:
    std::vector<std::unique_ptr<LogicalAnd>> m_ands{};
};

class Statement : public Node {};

class Scope : public Statement {};
class ElseStmt : public Node {};

class Program : public Node {
public:
    Program(std::vector<std::unique_ptr<Statement>> statements) 
        : m_statements(std::move(statements)) {}
private:
    std::vector<std::unique_ptr<Statement>> m_statements{};
};


class IfStmt : public Statement {
public:
    IfStmt(std::unique_ptr<Expression> expr, std::unique_ptr<Scope> scope,
            std::unique_ptr<ElseStmt> elseStmt)
        : m_expression(std::move(expr)), m_scope(std::move(scope))
        , m_else(std::move(elseStmt)) {}
private:
    std::unique_ptr<Expression> m_expression;
    std::unique_ptr<Scope> m_scope;
    std::unique_ptr<ElseStmt> m_else;
};

class WhileStmt : public Statement {};
class VarOrFuncDecl : public Statement {};
class VoidFuncDecl : public Statement {};
class RetStmt : public Statement {};
class IdArrFuncCall : public Statement {};




// class NodeVisitor : public Visitor {
// 
// };

class Parser {
public:
    Parser(ILexer& lexer) : m_lexer(lexer)
    {
        currToken = m_lexer.getToken();
        while (currToken.type == TokenType::COMMENT_T)
            currToken = m_lexer.getToken();
    }

    std::unique_ptr<Program> parseProgram() {
        std::vector<std::unique_ptr<Statement>> statements{};

        while (auto st = parseStatement())
            statements.push_back(std::move(st));


        return std::make_unique<Program>(std::move(statements));
    }

private:
    ILexer& m_lexer;
    Token prevToken{}, currToken{};
    void error(std::string_view message, Position tokenPos) {}

    Token peek() const { return currToken; }
    Token previous() const { return prevToken; }

    std::unique_ptr<Statement> parseStatement() {

        // Ignore lines that are pure newline
        while (match({TokenType::NEWLINE_T}));

        // I'll parse the trailing newline in the methods below (not outside like EBNF suggests)
        // TODO: try-catch
        if (auto st = parseIfStmt())        
            return st;
        if (auto st = parseWhileStmt())        
            return st;
        if (auto st = parseScope())        
            return st;
        if (auto st = parseVarOrFuncDecl())        
            return st;
        if (auto st = parseVoidFuncDecl())        
            return st;
        if (auto st = parseRetStmt())
            return st;
        if (auto st = parseIdArrFuncCall())
            return st;

        error("Invalid statement", peek().position);
        return nullptr;
    }

    std::unique_ptr<IfStmt> parseIfStmt() {
        if (!check(TokenType::IF_T)) 
            return nullptr;
        advance();

        if (!match({TokenType::L_BRACKET_T}))
            error("Missing left bracket", peek().position);

        auto condition = parseExpression();
        if (!condition) 
            throw SyntaxError("Invalid condition", peek().position);    
        
        if (!match({TokenType::L_BRACKET_T}))
            error("Missing right bracket", peek().position);

        match({TokenType::NEWLINE_T});

        auto scope = parseScope();
        if (!scope) 
            throw SyntaxError("Ill-formed scope", peek().position);

        if (!match({TokenType::NEWLINE_T}))
            error("Missing newline", peek().position);

        // TODO: add else statement support
        return std::make_unique<IfStmt>(std::move(condition), std::move(scope), nullptr);
    }
    std::unique_ptr<WhileStmt> parseWhileStmt();

    std::unique_ptr<Scope> parseScope() {
        if (!check(TokenType::L_BRACE_T))
            return nullptr;
        advance();

        return nullptr;
    }
    std::unique_ptr<VarOrFuncDecl> parseVarOrFuncDecl();
    std::unique_ptr<VoidFuncDecl> parseVoidFuncDecl();
    std::unique_ptr<RetStmt> parseRetStmt();
    std::unique_ptr<IdArrFuncCall> parseIdArrFuncCall();

    std::unique_ptr<Expression> parseExpression();
    
    bool isAtEnd() const { return peek().type == TokenType::EOT; }

    Token advance() 
    { 
        if (isAtEnd()) return prevToken;

        prevToken = currToken;        
        currToken = m_lexer.getToken();

        while (currToken.type == TokenType::COMMENT_T) {
            if (isAtEnd()) return prevToken;
            currToken = m_lexer.getToken();
        }

        return prevToken;
    }

    bool check(TokenType type) const {
        if (isAtEnd()) return false;
        return peek().type == type;
    }

    bool match(std::initializer_list<TokenType> types) {
        // If the token is in the types list, consume it
        for (TokenType type : types) {
            if (check(type)) {
                advance();
                return true;
            }
        }
        return false;
    }


    
};

#endif
