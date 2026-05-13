#ifndef _PARSER_H
#define _PARSER_H

#include <array>
#include <initializer_list>
#include <memory>
#include <string>
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

    // Czy to na prawdę najlepszy sposób na tablicę lambd?
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
        table[to_idx(TokenType::STR_T)] = makeUnaryOpFactory<StrCast>();
        table[to_idx(TokenType::INT_T)] = makeUnaryOpFactory<IntCast>();
        table[to_idx(TokenType::BOOL_T)] = makeUnaryOpFactory<BoolCast>();
        table[to_idx(TokenType::FLP_T)] = makeUnaryOpFactory<FlpCast>();

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
    
    static std::unordered_map<std::string_view, Position> functionMap;


    void error(std::string_view message, Position tokenPos) {}

    Token peek() const { return currToken; }
    Token previous() const { return prevToken; }

    void synchronize();

    std::unique_ptr<Statement> parseStatement(); 
    std::unique_ptr<Statement> parseScope();
    std::unique_ptr<Statement> parseScopedStmt();
    std::unique_ptr<Statement> parseVarDecl();
    std::unique_ptr<Statement> parseIfStmt();
    std::unique_ptr<Expression> parseCondition();
    std::unique_ptr<Statement> parseElseBody();
    std::unique_ptr<Statement> parseWhileStmt();
    std::unique_ptr<Statement> parseVarOrFuncDecl();
    std::unique_ptr<Statement> parseVoidFuncDecl();
    std::unique_ptr<Statement> parseFuncDecl(TypeInfo type, std::string name, Position pos); 
    std::unique_ptr<Statement> parseVarDeclTail(TypeInfo type, std::string name, Position pos);
    std::vector<Parameter> parseParameters(); 
    std::optional<TypeInfo> parseType(); 
    std::unique_ptr<Statement> parseRetStmt(); 
    std::unique_ptr<Statement> parseIdArrFunCall(); 

    std::unique_ptr<Expression> parseExpression(); 
    std::unique_ptr<Expression> parseAndExpr();
    std::unique_ptr<Expression> parseEqualityExpr() {
        auto leftFactor = parseRelationalExpr();
        if (!leftFactor)
            return nullptr;

        if (match({TokenType::EQ_T, TokenType::NOT_EQ_T})) {
            TokenType eqType = previous().type;
            Position eqPosition = previous().position;
            auto rightFactor = parseRelationalExpr();
            if (!rightFactor)
                throw SyntaxError("Missing expression after equality operator", peek().position);
            leftFactor = binaryOpTypeToObject[to_idx(eqType)](std::move(leftFactor), eqPosition, std::move(rightFactor));
        }

        return leftFactor;
    }

    std::unique_ptr<Expression> parseRelationalExpr() {
        auto leftFactor = parseArrayOpsExpr();
        if (!leftFactor)
            return nullptr;

        if (match({TokenType::LESSER_T, TokenType::LESSER_EQ_T, TokenType::GREATER_T, TokenType::GREATER_EQ_T})) {
            TokenType relType = previous().type;
            Position eqPosition = previous().position;
            auto rightFactor = parseArrayOpsExpr();
            if (!rightFactor)
                throw SyntaxError("Missing expression after inequality operator", peek().position);
            leftFactor = binaryOpTypeToObject[to_idx(relType)](std::move(leftFactor), eqPosition, std::move(rightFactor));
        }

        return leftFactor;
    }

    std::unique_ptr<Expression> parseArrayOpsExpr() {
        auto leftFactor = parseAdditiveExpr();
        if(!leftFactor)
            return nullptr;

        while (match({TokenType::CONCAT_T, TokenType::CONJUN_T, TokenType::SPLIT_T, TokenType::APPEND_T, TokenType::EXTRACT_T})) {
            TokenType arrOpType = previous().type;
            Position arrOpPosition = previous().position;
            auto rightFactor = parseAdditiveExpr();
            if (!rightFactor)
                throw SyntaxError("Missing expression after array operator", peek().position);
            leftFactor = binaryOpTypeToObject[to_idx(arrOpType)](std::move(leftFactor), arrOpPosition, std::move(rightFactor));
        }

        return leftFactor;
    }

    std::unique_ptr<Expression> parseAdditiveExpr() {
        auto leftFactor = parseMultiplExpr();
        if (!leftFactor)
            return nullptr;
        
        while (match({TokenType::PLUS_T, TokenType::MINUS_T})) {
            TokenType addType = previous().type;
            Position addPosition = previous().position;
            auto rightFactor = parseMultiplExpr();
            if (!rightFactor)
                throw SyntaxError("Missing expression after additive operator", peek().position);
            leftFactor = binaryOpTypeToObject[to_idx(addType)](std::move(leftFactor), addPosition, std::move(rightFactor));
        }

        return leftFactor;
    }

    std::unique_ptr<Expression> parseMultiplExpr() {
        auto leftFactor = parseUnaryExpr();
        if (!leftFactor)
            return nullptr;

        while (match({TokenType::MULT_T, TokenType::DIV_T, TokenType::MOD_T})) {
            TokenType multType = previous().type;
            Position multPosition = previous().position;
            auto rightFactor = parseMultiplExpr();
            if (!rightFactor)
                throw SyntaxError("Missing experssion after multiplicative operator", peek().position);
            leftFactor = binaryOpTypeToObject[to_idx(multType)](std::move(leftFactor), multPosition, std::move(rightFactor));
        }

        return leftFactor;
    }

    std::unique_ptr<Expression> parseUnaryExpr() {
        if (match({TokenType::PLUS_T, TokenType::MINUS_T, TokenType::NOT_T})) {
            TokenType unaryType = previous().type;
            Position unaryPosition = previous().position;
            auto factor = parsePostfix();
            if (!factor) 
                throw SyntaxError("Stray operator", unaryPosition);
            return unaryOpTypeToObject[to_idx(unaryType)](std::move(factor), unaryPosition); 
        }
        
        auto factor = parsePostfix();
        if (!factor)
            return nullptr;

        return factor;
    }

    std::unique_ptr<Expression> parsePostfix() {
        auto factor = parseTypeCast();
        if (!factor)
            return nullptr;

        if (match(TokenType::CARDINALITY_T))
            factor = unaryOpTypeToObject[to_idx(TokenType::CARDINALITY_T)](std::move(factor), previous().position);

        return factor;
    }

    std::unique_ptr<Expression> parseTypeCast() {
        auto factor = parseArrayExpr();
        if (!factor)
            return nullptr;

        while (match(TokenType::AS_T)) {
            Position asPosition = previous().position;

            if (match({TokenType::STR_T, TokenType::INT_T, TokenType::BOOL_T, TokenType::FLP_T})) {
                TokenType type = previous().type;
                factor = unaryOpTypeToObject[to_idx(type)](std::move(factor), asPosition);
            }
            else 
                throw SyntaxError("Invalid type in type cast", peek().position);
        }

        return factor;
    }

    std::unique_ptr<Expression> parseArrayExpr() {
        auto arrObj = parseSubject();
        if (!arrObj)
            return nullptr;

        while (match(TokenType::L_SQUARE_T)) {
            Position squarePos = previous().position;
            auto indexExpr = parseExpression();
            if (!indexExpr)
                throw SyntaxError("Missing or invalid array index/predicate inside square brackets", previous().position);
            arrObj = std::make_unique<ArrayExpr>(std::move(arrObj), squarePos, std::move(indexExpr));

            if (!match(TokenType::R_SQUARE_T))
                error("Missing closing square bracket", peek().position);
        }

        return arrObj;
    }

    std::unique_ptr<Expression> parseSubject() {
        if (auto exp = parseIdOrFunCall())
            return exp;
        if (auto exp = parseLiterals())
            return exp;
        if (auto exp = parseNestedExpr())
            return exp;
          
        throw SyntaxError("Invalid expression", peek().position);
    } 

    std::unique_ptr<Expression> parseIdOrFunCall() {
        if (!match(TokenType::IDENTIFIER_T))
            return nullptr;

        std::string idName = std::get<std::string>(previous().value);
        Position idPosition = previous().position;

        if (auto exp = parseFunCall(idName, idPosition))
            return exp;

        return std::make_unique<Identifier>(idName, idPosition);
    } 

    std::unique_ptr<Expression> parseFunCall(std::string name, Position position) {
        if (!match(TokenType::L_BRACKET_T))
            return nullptr;
        
        std::vector<std::unique_ptr<Expression>> arguments {};
        auto argument = parseExpression();
        if (argument) {
            arguments.push_back(std::move(argument));

            while (match(TokenType::COMMA_T)) {
                Position argPos = peek().position;
                argument = parseExpression();
                if (!argument)
                    throw SyntaxError("Invalid argument", argPos); 
                arguments.push_back(std::move(argument));
            }
        }

        if (!match(TokenType::R_BRACKET_T))
            error("Missing closing bracket", peek().position);

        return std::make_unique<FunCall>(name, std::move(arguments), position);
    }

    std::unique_ptr<Expression> parseLiterals() {
        if (match(TokenType::INT_VALUE_T)) 
            return std::make_unique<IntLit>(std::get<int>(previous().value), previous().position);
        if (match(TokenType::FLP_VALUE_T))
            return std::make_unique<FlpLit>(std::get<double>(previous().value), previous().position);
        if (match(TokenType::STR_VALUE_T))
            return std::make_unique<StrLit>(std::get<std::string>(previous().value), previous().position);
        if (match(TokenType::TRUE_T))
            return std::make_unique<BoolLit>(true, previous().position);
        if (match(TokenType::FALSE_T))
            return std::make_unique<BoolLit>(false, previous().position);
        if (auto arr = parseArrayLiteral())
            return arr;

        return nullptr; 
    } 

    std::unique_ptr<Expression> parseArrayLiteral() {
        if (!match(TokenType::L_SQUARE_T)) 
            return nullptr;

        Position arrPos = previous().position;
        std::vector<std::unique_ptr<Expression>> values {};
        auto value = parseExpression();
        if (value) {
            values.push_back(std::move(value));
            
            while (match(TokenType::COMMA_T)) {
                Position valPos = peek().position;
                value = parseExpression();
                if (!value)
                    throw SyntaxError("Invalid expression in array literal", valPos);
                values.push_back(std::move(value));
            }
        }

        if (!match(TokenType::R_SQUARE_T))
            error("Missing closing square bracket in array literal", peek().position);

        return std::make_unique<ArrayLit>(std::move(values), arrPos);
    }

    std::unique_ptr<Expression> parseNestedExpr() {
        if (!match(TokenType::L_BRACKET_T))
            return nullptr;
        
        auto expr = parseExpression();

        if (!match(TokenType::R_BRACKET_T))
            error("Missing closing parenthesis", peek().position);

        return expr;
    }

    
    bool isAtEnd() const { return peek().type == TokenType::EOT; }

    void advance() 
    { 
        if (isAtEnd()) return;  // = prevToken;

        prevToken = currToken;        
        currToken = m_lexer.getToken();

        while (currToken.type == TokenType::COMMENT_T) {
            if (isAtEnd()) return; // = prevToken;
            currToken = m_lexer.getToken();
        }

        return; // = prevToken;
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
