#ifndef _PARSER_H
#define _PARSER_H

#include <array>
#include <memory>
#include <string>
#include <sys/syslimits.h>
#include <utility>
#include <vector>
#include "Lexer.h"
#include "Token.h"
#include "Node.h"
#include <ErrorHandler.h>
 
template <typename T>
constexpr auto makeBinOpFactory() {
    return [](std::unique_ptr<Expression> l, Position p, std::unique_ptr<Expression> r) -> std::unique_ptr<Expression> {
        return std::make_unique<T>(std::move(l), p, std::move(r));
    };
}

template <typename T>
constexpr auto makeUnaryOpFactory() {
    return [](std::unique_ptr<Expression> l, Position p) -> std::unique_ptr<Expression> {
        return std::make_unique<T>(std::move(l), p);
    };
}

template <typename T>
constexpr auto makeAssignFactory() {
    return [](std::unique_ptr<Expression> l, Position p, std::unique_ptr<Expression> r) -> std::unique_ptr<Statement> {
        return std::make_unique<T>(std::move(l), p, std::move(r));
    };
}

class Parser {
public:
    Parser(ILexer& lexer, ErrorHandler& errorHandler) 
        : m_lexer(lexer), m_errorHandler(errorHandler)
    {
        getToken();
    }

    Program parse();

private:
    ILexer& m_lexer;
    ErrorHandler& m_errorHandler;
    Token prevToken{}, currToken{};

    // For use in tokenType -> Constructor lambdas (see createBinOpTable())
    using ExprPtr = std::unique_ptr<Expression>;
    using BinOpFactory = ExprPtr(*)(ExprPtr, Position, ExprPtr);
    using UnaryOpFactory = ExprPtr(*)(ExprPtr, Position);
    using AssignFactory = std::unique_ptr<Statement>(*)(ExprPtr, Position, ExprPtr);


    // Binary operator type to class pointer table creation
    static std::array<BinOpFactory, std::to_underlying(TokenType::UNKNOWN)> createBinOpTable();
    static std::array<UnaryOpFactory, std::to_underlying(TokenType::UNKNOWN)> createUnaryOpTable();
    static std::array<AssignFactory, std::to_underlying(TokenType::UNKNOWN)> createAssignTable();
    static inline const auto binaryOpTypeToObject = createBinOpTable();
    static inline const auto unaryOpTypeToObject = createUnaryOpTable();
    static inline const auto assTypeToObject = createAssignTable();

    Token peek() const { return currToken; }
    Token previous() const { return prevToken; }

    void synchronize();

    // Statement productions
    std::unique_ptr<Statement> parseStatement(); 
    std::unique_ptr<Statement> parseScope();
    std::unique_ptr<Statement> parseScopedStmt();
    std::unique_ptr<Statement> parseVarDecl();
    std::unique_ptr<Statement> parseIfStmt();
    std::unique_ptr<Statement> parseIfTail();
    std::unique_ptr<Expression> parseCondition();
    std::unique_ptr<Statement> parseElseStmt(); 
    std::unique_ptr<Statement> parseElseBody();
    std::unique_ptr<Statement> parseWhileStmt();
    std::unique_ptr<Statement> parseVarOrFuncDecl();
    std::unique_ptr<Statement> parseVoidFuncDecl();
    std::unique_ptr<Statement> parseFuncDecl(TypeInfo type, std::string name, Position pos); 
    std::unique_ptr<Statement> parseVarDeclAssign(TypeInfo type, std::string name, Position pos);
    std::unique_ptr<Statement> parseRetStmt(); 
    std::unique_ptr<Statement> parseIdArrFunCall(); 
    std::unique_ptr<Expression> parseArrayIdx(std::string name, Position pos);
    std::unique_ptr<Statement> parseAssign(std::string name, Position pos);

    // Helper productions
    std::vector<Parameter> parseParameters(); 
    std::optional<TypeInfo> parseType(); 

    // Expression productions
    std::unique_ptr<Expression> parseExpression(); 
    std::unique_ptr<Expression> parseAndExpr();
    std::unique_ptr<Expression> parseEqualityExpr(); 
    std::unique_ptr<Expression> parseRelationalExpr(); 
    std::unique_ptr<Expression> parseArrayOpsExpr(); 
    std::unique_ptr<Expression> parseAdditiveExpr(); 
    std::unique_ptr<Expression> parseMultiplExpr(); 
    std::unique_ptr<Expression> parseUnaryExpr();
    std::unique_ptr<Expression> parsePostfix(); 
    std::unique_ptr<Expression> parseTypeCast(); 
    std::unique_ptr<Expression> parseArrayExpr(); 
    std::unique_ptr<Expression> parseSubject(); 
    std::unique_ptr<Expression> parseIdOrFunCall(); 
    std::unique_ptr<Expression> parseFunCall(std::string name, Position position); 
    std::vector<std::unique_ptr<Expression>> parseArguments();
    std::unique_ptr<Expression> parseLiterals(); 
    std::unique_ptr<Expression> parseArrayLiteral(); 
    std::unique_ptr<Expression> parseNestedExpr(); 
    
    bool isAtEnd() const { return peek().type == TokenType::EOT; }

    void getToken() {
        currToken = m_lexer.getToken();
        while (currToken.type == TokenType::COMMENT_T)
            currToken = m_lexer.getToken();
    }

    void advance() 
    { 
        if (isAtEnd()) return;  

        prevToken = currToken;        
        getToken();
    }

    bool check(TokenType type) const {
        if (isAtEnd()) return false;
        return peek().type == type;
    }

    template<typename... Args>
    bool match(Args... types) {
        if (((check(types)) || ...)) {
            advance();
            return true;
        }
        return false;
    }

    template <typename T>
    void throwIfMissing(const T& arg, 
            const std::string& message, 
            std::optional<Position> pos = std::nullopt) {

        if (!arg) {
            Position position = pos.value_or(peek().position);
            m_errorHandler.report(std::make_unique<SyntaxError>(message, Severity::ERROR, position));
        }
    }

    void consume(TokenType type, const std::string& message) {
        if (!match(type))
            m_errorHandler.report(std::make_unique<SyntaxError>(message, Severity::WARNING, peek().position));
    }

    std::string consumeIdentifier() {
        throwIfMissing(match(TokenType::IDENTIFIER_T), "Expected identifier name after type");
        return std::get<std::string>(previous().value);
    }
};

#endif
