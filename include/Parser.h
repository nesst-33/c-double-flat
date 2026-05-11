#ifndef _PARSER_H
#define _PARSER_H

#include <algorithm>
#include <array>
#include <functional>
#include <initializer_list>
#include <memory>
#include "Lexer.h"
#include "Token.h"
#include "Node.h"

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

// Do zamiany enum na size_t
template <typename T>
constexpr auto to_idx(T e) {
    return static_cast<std::size_t>(e);
}

class Parser {
public:
    Parser(ILexer& lexer) : m_lexer(lexer)
    {
        currToken = m_lexer.getToken();
        while (currToken.type == TokenType::COMMENT_T)
            currToken = m_lexer.getToken();
    }

    // For use in binaryOpTable
    using ExprPtr = std::unique_ptr<Expression>;
    using FactoryFunc = ExprPtr(*)(ExprPtr, ExprPtr);

    std::unique_ptr<Program> parseProgram() {
        std::vector<std::unique_ptr<Statement>> statements{};

        while (auto st = parseStatement())
            statements.push_back(std::move(st));


        return std::make_unique<Program>(std::move(statements));
    }

private:
    ILexer& m_lexer;
    Token prevToken{}, currToken{};

    // Czy to na prawdę najlepszy sposób na tablicę lambd?
    // Binary operator type to class pointer table creation
    static constexpr std::array<FactoryFunc, to_idx(TokenType::LESSER_EQ_T) + 1> createOpTable() {
        std::array<FactoryFunc, to_idx(TokenType::LESSER_EQ_T) + 1> table{};

        table[to_idx(TokenType::PLUS_T)] = [] (ExprPtr l, ExprPtr r) -> ExprPtr { return std::make_unique<AddExpr>(std::move(l), std::move(r)); };
        table[to_idx(TokenType::MINUS_T)] = [] (ExprPtr l, ExprPtr r) -> ExprPtr { return std::make_unique<SubExpr>(std::move(l), std::move(r)); };
        table[to_idx(TokenType::MULT_T)] = [] (ExprPtr l, ExprPtr r) -> ExprPtr { return std::make_unique<MultExpr>(std::move(l), std::move(r)); };
        table[to_idx(TokenType::DIV_T)] = [] (ExprPtr l, ExprPtr r) -> ExprPtr { return std::make_unique<DivExpr>(std::move(l), std::move(r)); };
        table[to_idx(TokenType::MOD_T)] = [] (ExprPtr l, ExprPtr r) -> ExprPtr { return std::make_unique<ModExpr>(std::move(l), std::move(r)); };
        table[to_idx(TokenType::CONCAT_T)] = [] (ExprPtr l, ExprPtr r) -> ExprPtr { return std::make_unique<ConcatExpr>(std::move(l), std::move(r)); };
        table[to_idx(TokenType::CONJUN_T)] = [] (ExprPtr l, ExprPtr r) -> ExprPtr { return std::make_unique<ConjunExpr>(std::move(l), std::move(r)); };
        table[to_idx(TokenType::SPLIT_T)] = [] (ExprPtr l, ExprPtr r) -> ExprPtr { return std::make_unique<SplitExpr>(std::move(l), std::move(r)); };
        table[to_idx(TokenType::APPEND_T)] = [] (ExprPtr l, ExprPtr r) -> ExprPtr { return std::make_unique<AppendExpr>(std::move(l), std::move(r)); };
        table[to_idx(TokenType::EXTRACT_T)] = [] (ExprPtr l, ExprPtr r) -> ExprPtr { return std::make_unique<ExtractExpr>(std::move(l), std::move(r)); };
        table[to_idx(TokenType::GREATER_T)] = [] (ExprPtr l, ExprPtr r) -> ExprPtr { return std::make_unique<GreatExpr>(std::move(l), std::move(r)); };
        table[to_idx(TokenType::LESSER_T)] = [] (ExprPtr l, ExprPtr r) -> ExprPtr { return std::make_unique<LessExpr>(std::move(l), std::move(r)); };
        table[to_idx(TokenType::EQ_T)] = [] (ExprPtr l, ExprPtr r) -> ExprPtr { return std::make_unique<EqExpr>(std::move(l), std::move(r)); };
        table[to_idx(TokenType::NOT_EQ_T)] = [] (ExprPtr l, ExprPtr r) -> ExprPtr { return std::make_unique<NotEqExpr>(std::move(l), std::move(r)); };
        table[to_idx(TokenType::GREATER_EQ_T)] = [] (ExprPtr l, ExprPtr r) -> ExprPtr { return std::make_unique<GreatEqExpr>(std::move(l), std::move(r)); };
        table[to_idx(TokenType::LESSER_EQ_T)] = [] (ExprPtr l, ExprPtr r) -> ExprPtr { return std::make_unique<LessEqExpr>(std::move(l), std::move(r)); };
        return table;
    }

    static constexpr auto binaryOpTable = createOpTable();

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

        // TODO: dodać słownik typu <string, FuncDecl>, który będzie przechowywał zadeklarowane funkcje
        // jeśli deklaracja się powtórzy, to błąd
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

    std::unique_ptr<Expression> parseExpression() {
        auto leftFactor = parseAndExpr();
        if (!leftFactor)
            return nullptr; 

        while(match({TokenType::OR_T}))
        {
            Position orPosition = previous().position;
            auto rightFactor = parseAndExpr();
            if (!rightFactor)
                throw SyntaxError("Missing expression after or keyword", peek().position);

            leftFactor = std::make_unique<OrExpr>(std::move(leftFactor),
                    orPosition, std::move(rightFactor));
        }
        return leftFactor;
    }

    std::unique_ptr<Expression> parseAndExpr() {
        auto leftFactor = parseEqualityExpr();
        if (!leftFactor)
            return nullptr;

        while(match(TokenType::AND_T)) {
            Position andPosition = previous().position;
            auto rightFactor = parseEqualityExpr();
            if (!rightFactor)
                throw SyntaxError("Missing expression after and keyword", peek().position);

            leftFactor = std::make_unique<AndExpr>(std::move(leftFactor),
                    andPosition, std::move(rightFactor));
        }

        return leftFactor;
    }

    std::unique_ptr<Expression> parseEqualityExpr() {
        auto leftFactor = parseRelationalExpr();
        if (!leftFactor)
            return nullptr;

        return nullptr;
    }

    std::unique_ptr<Expression> parseRelationalExpr() { return nullptr; }
    
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

    bool match(TokenType type) {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
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
