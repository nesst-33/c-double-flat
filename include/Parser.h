#ifndef _PARSER_H
#define _PARSER_H

#include <array>
#include <initializer_list>
#include <memory>
#include <utility>
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

class Parser;

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

    static inline const auto binaryOpTypeToObject = createBinOpTable();
    static inline const auto unaryOpTypeToObject = createUnaryOpTable();

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
                throw SyntaxError("Missing expression after 'or' keyword", peek().position);

            leftFactor = binaryOpTypeToObject[to_idx(TokenType::OR_T)](std::move(leftFactor), orPosition, std::move(rightFactor));
        }
        return leftFactor;
    }

    // Uogólnić do parseBinaryOp()??
    std::unique_ptr<Expression> parseAndExpr() {
        auto leftFactor = parseEqualityExpr();
        if (!leftFactor)
            return nullptr;

        while(match(TokenType::AND_T)) {
            Position andPosition = previous().position;
            auto rightFactor = parseEqualityExpr();
            if (!rightFactor)
                throw SyntaxError("Missing expression after 'and' keyword", peek().position);

            leftFactor = binaryOpTypeToObject[to_idx(TokenType::AND_T)](std::move(leftFactor), andPosition, std::move(rightFactor));
        }

        return leftFactor;
    }

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
        return nullptr;
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
