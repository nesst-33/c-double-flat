#ifndef _PARSER_H
#define _PARSER_H

#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <sys/syslimits.h>
#include <utility>
#include <vector>
#include <unordered_map>
#include <format>
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
    const Position& getPosition() const { return m_pos; }
private:
    Position m_pos;
};

// Do zamiany enum na size_t
template <typename T>
constexpr auto to_idx(T e) {
    return static_cast<std::size_t>(e);
}

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
    Parser(ILexer& lexer) : m_lexer(lexer)
    {
        currToken = m_lexer.getToken();
        while (currToken.type == TokenType::COMMENT_T)
            currToken = m_lexer.getToken();
    }

    // For use in tokenType -> Constructor lambdas (see createBinOpTable())
    using ExprPtr = std::unique_ptr<Expression>;
    using BinOpFactory = ExprPtr(*)(ExprPtr, Position, ExprPtr);
    using UnaryOpFactory = ExprPtr(*)(ExprPtr, Position);
    using AssignFactory = std::unique_ptr<Statement>(*)(ExprPtr, Position, ExprPtr);

    std::unique_ptr<Program> parse();

private:
    ILexer& m_lexer;
    Token prevToken{}, currToken{};


    // Binary operator type to class pointer table creation
    static constexpr std::array<BinOpFactory, to_idx(TokenType::UNKNOWN)> createBinOpTable() {
        std::array<BinOpFactory, to_idx(TokenType::UNKNOWN)> table{};

        table[to_idx(TokenType::PLUS_T)] = makeBinOpFactory<AddExpr>(); 
        table[to_idx(TokenType::MINUS_T)] = makeBinOpFactory<SubExpr>(); 
        table[to_idx(TokenType::MULT_T)] = makeBinOpFactory<MultExpr>(); 
        table[to_idx(TokenType::DIV_T)] = makeBinOpFactory<DivExpr>(); 
        table[to_idx(TokenType::MOD_T)] = makeBinOpFactory<ModExpr>(); 
        table[to_idx(TokenType::CONCAT_T)] = makeBinOpFactory<ConcatExpr>(); 
        table[to_idx(TokenType::CONJUN_T)] = makeBinOpFactory<ConjunExpr>(); 
        table[to_idx(TokenType::SPLIT_T)] = makeBinOpFactory<SplitExpr>(); 
        table[to_idx(TokenType::APPEND_T)] = makeBinOpFactory<AppendExpr>(); 
        table[to_idx(TokenType::EXTRACT_T)] = makeBinOpFactory<ExtractExpr>(); 
        table[to_idx(TokenType::GREATER_T)] = makeBinOpFactory<GreatExpr>(); 
        table[to_idx(TokenType::LESSER_T)] = makeBinOpFactory<LessExpr>(); 
        table[to_idx(TokenType::EQ_T)] = makeBinOpFactory<EqExpr>(); 
        table[to_idx(TokenType::NOT_EQ_T)] = makeBinOpFactory<NotEqExpr>(); 
        table[to_idx(TokenType::GREATER_EQ_T)] = makeBinOpFactory<GreatEqExpr>(); 
        table[to_idx(TokenType::LESSER_EQ_T)] = makeBinOpFactory<LessEqExpr>(); 
        table[to_idx(TokenType::AND_T)] = makeBinOpFactory<AndExpr>(); 
        table[to_idx(TokenType::OR_T)] = makeBinOpFactory<OrExpr>(); 

        return table;
   }

    static constexpr std::array<UnaryOpFactory, to_idx(TokenType::UNKNOWN)> createUnaryOpTable() {
        std::array<UnaryOpFactory, to_idx(TokenType::UNKNOWN)> table{};

        table[to_idx(TokenType::PLUS_T)] = makeUnaryOpFactory<PositiveExpr>();
        table[to_idx(TokenType::MINUS_T)] = makeUnaryOpFactory<NegativeExpr>();
        table[to_idx(TokenType::NOT_T)] = makeUnaryOpFactory<NotExpr>();
        table[to_idx(TokenType::CARDINALITY_T)] = makeUnaryOpFactory<CardinalityExpr>();

        return table;
    }

    static constexpr std::array<AssignFactory, to_idx(TokenType::UNKNOWN)> createAssignTable() {
        std::array<AssignFactory, to_idx(TokenType::UNKNOWN)> table{}; 

        table[to_idx(TokenType::ASSIGN_T)] = makeAssignFactory<BasicAssignStmt>();
        table[to_idx(TokenType::ADD_ASSIGN_T)] = makeAssignFactory<AddAssignStmt>();
        table[to_idx(TokenType::SUB_ASSIGN_T)] = makeAssignFactory<SubAssignStmt>();
        table[to_idx(TokenType::MULT_ASSIGN_T)] = makeAssignFactory<MultAssignStmt>();
        table[to_idx(TokenType::DIV_ASSIGN_T)] = makeAssignFactory<DivAssignStmt>();
        table[to_idx(TokenType::MOD_ASSIGN_T)] = makeAssignFactory<ModAssignStmt>();
        table[to_idx(TokenType::CONCAT_ASSIGN_T)] = makeAssignFactory<ConcatAssignStmt>();

        return table;
    }

    static inline const auto binaryOpTypeToObject = createBinOpTable();
    static inline const auto unaryOpTypeToObject = createUnaryOpTable();
    static inline const auto assTypeToObject = createAssignTable();
    
    void error(std::string_view message, Position tokenPos) {
        std::cout << message << " at column " << tokenPos.column << ", line "
            << tokenPos.line << " (char offset: " << tokenPos.offset << ").\n";
    }

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

    void advance() 
    { 
        if (isAtEnd()) return;  

        prevToken = currToken;        
        currToken = m_lexer.getToken();

        while (currToken.type == TokenType::COMMENT_T) {
            if (isAtEnd()) return; 
            currToken = m_lexer.getToken();
        }

        return; 
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
    void throwIfMissing(const T& arg, const std::string& message, std::optional<Position> pos = std::nullopt) {
        if (!arg)
            throw SyntaxError(message, pos.value_or(peek().position));
    }

    void consume(TokenType type, std::string_view message) {
        if (!match(type))
            error(message, peek().position);
    }




    
};

#endif
