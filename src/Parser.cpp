#include "Parser.h"
#include "Token.h"
#include <algorithm>
#include <memory>
#include <vector>


// program = { statement }, EOT ;
std::unique_ptr<Program> Parser::parse() {
    std::vector<std::unique_ptr<Statement>> statements{};
    while (auto statement = parseStatement())
        statements.push_back(std::move(statement));
    if (!isAtEnd())
        throw SyntaxError("Expected end of file", peek().position);

    return std::make_unique<Program>(std::move(statements));
}

void Parser::synchronize() {
    while (!isAtEnd()) {
        switch(peek().type) {
            case TokenType::IF_T:
            case TokenType::WHILE_T:
            case TokenType::L_BRACE_T:
            case TokenType::INT_T:
            case TokenType::FLP_T:
            case TokenType::STR_T:
            case TokenType::BOOL_T:
            case TokenType::VOID_T:
            case TokenType::ARR_T:
            case TokenType::IDENTIFIER_T:
            case TokenType::RETURN_T:
                return;
            default:
                continue;
        }
        advance();
    }
}

// statement = if_stmt | while_stmt | scope_stmt | var_or_func_decl
//           | void_func_decl | id_arr_func_call | newline
std::unique_ptr<Statement> Parser::parseStatement() {
    
    // Ignore lines that are pure newline
    while (match(TokenType::NEWLINE_T));

    // TODO: move SyntaxError do parse()
    try {
        if (auto st = parseIfStmt())        
            return st;
        if (auto st = parseWhileStmt())        
            return st;
        if (auto st = parseScope()) {        
            if (!match(TokenType::NEWLINE_T)) 
                error("Missing terminating newline", peek().position);
            return st;
        }
        if (auto st = parseVarOrFuncDecl())
            return st;
        if (auto st = parseVoidFuncDecl())        
            return st;
        if (auto st = parseIdArrFunCall())
            return st;
    } catch (SyntaxError e) {
        error(std::format("Invalid statement: {}", e.what()), e.getPosition());
        synchronize();
    }

    return nullptr;
}

// scope =  "{", [newline], {scoped_stmt}, "}"
std::unique_ptr<Statement> Parser::parseScope() {
    if (!match(TokenType::L_BRACE_T))
        return nullptr;
    Position scopePos = previous().position;

    std::vector<std::unique_ptr<Statement>> statements {};
    while (!match(TokenType::R_BRACE_T)) {
        if (auto statement = parseScopedStmt())
            statements.push_back(std::move(statement));
    }
    return std::make_unique<Scope>(std::move(statements), scopePos);
}

// scoped_stmt = var_decl | scope_stmt | if_stmt | while_stmt | return_stmt
//             | id_arr_func_call | newline
std::unique_ptr<Statement> Parser::parseScopedStmt() {
    while(match(TokenType::NEWLINE_T));

    try {
        if (auto st = parseIfStmt())
            return st;
        if (auto st = parseWhileStmt())
            return st;
        if (auto st = parseScope()) {
            // TODO: change to method
            if (!match(TokenType::NEWLINE_T))
                error("Missing terminating newline", peek().position);
            return st;
        }
        if (auto st = parseIdArrFunCall())
            return st;
        if (auto st = parseVarDecl())
            return st;
        if (auto st = parseRetStmt())
            return st;
    } catch (SyntaxError e) {
        error(std::format("Syntax error: {}", e.what()), e.getPosition());
        synchronize();
    }

    return nullptr;
}

// var_decl = type, identifier, ["=", expression], newline;
std::unique_ptr<Statement> Parser::parseVarDecl() {
    Position startPos = peek().position;
    auto typeOpt = parseType();
    if (!typeOpt)
        return nullptr;
    TypeInfo type = *typeOpt;
    if (!match(TokenType::IDENTIFIER_T))
        throw SyntaxError("Expected identifier name after type", peek().position);

    std::string name = std::get<std::string>(previous().value);

    if (!match(TokenType::NEWLINE_T))
        error("Missing terminating newline", peek().position);
    
    return parseVarDeclAssign(type, name, startPos);
}

// if_stmt = "if", condition, [newline], scope, [ [newline], else_stmt ], newline;
std::unique_ptr<Statement> Parser::parseIfStmt() {
    if (!match(TokenType::IF_T))
        return nullptr;

    Position ifPos = previous().position;

    auto condition = parseCondition();

    match(TokenType::NEWLINE_T);

    auto scope = parseScope();
    if (!scope) 
        throw SyntaxError("Ill-formed scope", peek().position);

    auto elseStmt = parseIfTail();

    return std::make_unique<IfStmt>(std::move(condition), std::move(scope), std::move(elseStmt), ifPos);
}

// condition = "(", expression, ")"
std::unique_ptr<Expression> Parser::parseCondition() {
    if (!match(TokenType::L_BRACKET_T))
        error("Missing left bracket", peek().position);

    auto condition = parseExpression();
    if (!condition) 
        throw SyntaxError("Invalid condition", peek().position);    
    
    if (!match(TokenType::R_BRACKET_T))
        error("Missing right bracket", peek().position);

    return condition;
}

// if_tail = else_stmt | (newline, [else_stmt]) 
std::unique_ptr<Statement> Parser::parseIfTail() {
    auto elseStmt = parseElseStmt();
    if (!elseStmt) {
        if (!match(TokenType::NEWLINE_T))
            error("Expected 'else' or newline after if-statement scope", peek().position);
        elseStmt = parseElseStmt();
    }

    return elseStmt;
}

// else_stmt = else_body, newline
std::unique_ptr<Statement> Parser::parseElseStmt() {
    auto elseBody = parseElseBody();
    if (!elseBody)
        return nullptr;

    if (!match(TokenType::NEWLINE_T))
        error("Missing terminating newline", peek().position);

    return elseBody;
}

// else_body = "else", [newline], scope
std::unique_ptr<Statement> Parser::parseElseBody() {
    if (!match(TokenType::ELSE_T))
        return nullptr;

    match(TokenType::NEWLINE_T);

    auto elseBody = parseScope();
    if (!elseBody)
        throw SyntaxError("Invalid else body", peek().position);

    if (!match(TokenType::NEWLINE_T))
        error("Missing terminating newline", peek().position);

    return elseBody;
}

// while_stmt = "while", condition, [newline], scope, newline;
std::unique_ptr<Statement> Parser::parseWhileStmt() {
    if (!match(TokenType::WHILE_T))
        return nullptr;

    Position whilePos = previous().position;

    auto condition = parseCondition();

    match(TokenType::NEWLINE_T);

    auto whileBody = parseScope();
    if (!whileBody)
        throw SyntaxError("Missing while body", peek().position);

    if (!match(TokenType::NEWLINE_T))
        error("Missing terminating newline", peek().position);

    return std::make_unique<WhileStmt>(std::move(condition), std::move(whileBody), whilePos);
}

// var_or_func_decl = type, identifier, (func_declaration | [var_decl_assign]), newline
std::unique_ptr<Statement> Parser::parseVarOrFuncDecl() {
    Position startPos = peek().position;
    auto typeOpt = parseType();
    if (!typeOpt)
        return nullptr;

    TypeInfo type = *typeOpt;

    if (!match(TokenType::IDENTIFIER_T))
        throw SyntaxError("Expected identifier name after type", peek().position);

    std::string name = std::get<std::string>(previous().value);

    auto func = parseFuncDecl(type, name, startPos);
    if (!func)
        auto var = parseVarDeclAssign(type, name, startPos);
    
    if (!match(TokenType::NEWLINE_T))
        error("Missing terminating newline", peek().position);

    return (func ? func : var);
}

// void_func_decl = "void", identifier, func_declaration, newline
std::unique_ptr<Statement> Parser::parseVoidFuncDecl() {
    if (!match(TokenType::VOID_T))
        return nullptr;
    Position startPos = previous().position;
    TypeInfo type {BaseType::VOID};
    
    if (!match(TokenType::IDENTIFIER_T))
        throw SyntaxError("Expected identifier name after type", peek().position);

    std::string name = std::get<std::string>(previous().value);
    auto func = parseFuncDecl(type, name, startPos);
    if (!func)
        throw SyntaxError("Missing function declaration", peek().position);

    if (!match(TokenType::NEWLINE_T))
        error("Missing terminating newline", peek().position);

    return func;
}

// func_declaration = "(", [parameters], ")", [newline], scope 
std::unique_ptr<Statement> Parser::parseFuncDecl(TypeInfo type, std::string name, Position pos) {
    if (!match(TokenType::L_BRACKET_T))
        return nullptr;

    std::vector<Parameter> params = parseParameters();

    if (!match(TokenType::R_BRACKET_T))
        error("Missing closing bracket in parameter list", peek().position);
    
    match(TokenType::NEWLINE_T);

    auto body = parseScope();
    if (!body)
        throw SyntaxError("Missing function body", peek().position);

    // TODO: usunąć mapę
    if (auto search = functionMap.find(name); search == functionMap.end())
        functionMap.insert({name, pos});
    else
        throw SyntaxError("Function redefinition", pos);

    return std::make_unique<FuncDeclStmt>(type, name, std::move(params), std::move(body), pos);
}

// var_decl_assign = "=", expression
std::unique_ptr<Statement> Parser::parseVarDeclAssign(TypeInfo type, std::string name, Position pos) {
    std::unique_ptr<Expression> initializer;
    if (match(TokenType::ASSIGN_T)) {
        initializer = parseExpression();
        if (!initializer)
            throw SyntaxError("Invalid expression after '='", peek().position);
    }

    return std::make_unique<VarDeclStmt>(type, name, std::move(initializer), pos);
}

// parameters = type, identifier, {",", type, identifier}
std::vector<Parameter> Parser::parseParameters() {
    std::vector<Parameter> params{};
    auto typeOpt = parseType();
    if (!typeOpt)
        return params;

    TypeInfo type = *typeOpt;

    if (!match(TokenType::IDENTIFIER_T))
        throw SyntaxError("Expected identifier name after type", peek().position);

    params.push_back({std::move(type), std::get<std::string>(previous().value), previous().position});

    while (match(TokenType::COMMA_T)) {
        typeOpt = parseType();
        
        if (!typeOpt) {
            error("Trailing comma", previous().position);
            continue;
        }

        type = *typeOpt; 
        if (!match(TokenType::IDENTIFIER_T))
            throw SyntaxError("Expected identifier name after type", peek().position);
        Parameter param = {std::move(type), std::get<std::string>(previous().value), previous().position};
        params.push_back(param);
    }

    return params;
}

// type = ["const"], ["arr", {"arr"}], ("int", "str", "flp", "bool")
std::optional<TypeInfo> Parser::parseType() {
    TypeInfo type{};
    type.isConst = match(TokenType::CONST_T);

    while (match(TokenType::ARR_T))
        type.arrayDepth++;

    if (match(TokenType::INT_T))
        type.type = BaseType::INT;
    else if (match(TokenType::FLP_T))
        type.type = BaseType::FLP;
    else if (match(TokenType::STR_T))
        type.type = BaseType::STR;
    else if (match(TokenType::BOOL_T))
        type.type = BaseType::BOOL;
    else {
        if (type.isConst || type.arrayDepth > 0)
            throw SyntaxError("Missing type in declaration", peek().position);
        return std::nullopt;
    }

    return type;
}

// return_stmt = "return", [expression], newline
std::unique_ptr<Statement> Parser::parseRetStmt() {
    if (!match(TokenType::RETURN_T))
        return nullptr;

    Position retPos = previous().position;
    auto expression = parseExpression();

    if (!match(TokenType::NEWLINE_T))
        error("Missing terminating newline", peek().position);

    return std::make_unique<RetStmt>(std::move(expression), retPos);
}

// id_arr_func_call = identifier, (func_call | ( array_idx, assign ) | assign), newline
std::unique_ptr<Statement> Parser::parseIdArrFunCall() {
    if (!match(TokenType::IDENTIFIER_T))
        return nullptr;

    std::string name = std::get<std::string>(previous().value);
    Position pos = previous().position;

    // TODO: MISSING NEWLINE CHECK!!!
    if (auto funCall = parseFunCall(name, pos))
        return std::make_unique<FunCallStmt>(std::move(funCall), pos);

    std::unique_ptr<Expression> lhs = std::make_unique<Identifier>(name, pos);

    // Parse array indexing
    while (match(TokenType::L_SQUARE_T)) {
        Position squarePos = previous().position;
        auto indexExpr = parseExpression();

        if (!indexExpr)
            throw SyntaxError("Missing array index inside square brackets", previous().position);

        if (!match(TokenType::R_SQUARE_T))
            error("Missing closing square bracket", peek().position);

        lhs = std::make_unique<ArrayExpr>(std::move(lhs), squarePos, std::move(indexExpr));
    }
    
    // Parse assignment
    if (!match({TokenType::ASSIGN_T, TokenType::ADD_ASSIGN_T, TokenType::SUB_ASSIGN_T, TokenType::MULT_ASSIGN_T,
                TokenType::DIV_ASSIGN_T, TokenType::MOD_ASSIGN_T, TokenType::CONCAT_ASSIGN_T}))
        throw SyntaxError("Expected assignment or function call", peek().position);

    Position assPos = previous().position;
    TokenType assType = previous().type;
    auto rhs = parseExpression();
    if (!rhs)
        throw SyntaxError("Expected expression", peek().position);
    
    if (!match(TokenType::NEWLINE_T))
        error("Missing terminating newline", peek().position);

    return assTypeToObject[to_idx(assType)](std::move(lhs), assPos, std::move(rhs));
}

std::unique_ptr<Expression> Parser::parseExpression() {
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

std::unique_ptr<Expression> Parser::parseAndExpr() {
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

std::unique_ptr<Expression> Parser::parseEqualityExpr() {
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

std::unique_ptr<Expression> Parser::parseRelationalExpr() {
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

std::unique_ptr<Expression> Parser::parseArrayOpsExpr() {
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

std::unique_ptr<Expression> Parser::parseAdditiveExpr() {
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

std::unique_ptr<Expression> Parser::parseMultiplExpr() {
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

std::unique_ptr<Expression> Parser::parseUnaryExpr() {
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

std::unique_ptr<Expression> Parser::parsePostfix() {
    auto factor = parseTypeCast();
    if (!factor)
        return nullptr;

    if (match(TokenType::CARDINALITY_T))
        factor = unaryOpTypeToObject[to_idx(TokenType::CARDINALITY_T)](std::move(factor), previous().position);

    return factor;
}

std::unique_ptr<Expression> Parser::parseTypeCast() {
    auto factor = parseArrayExpr();
    if (!factor)
        return nullptr;

    while (match(TokenType::AS_T)) {
        Position asPosition = previous().position;

        // TODO: zamiast 4 klas 1
        if (match({TokenType::STR_T, TokenType::INT_T, TokenType::BOOL_T, TokenType::FLP_T})) {
            TokenType type = previous().type;
            factor = unaryOpTypeToObject[to_idx(type)](std::move(factor), asPosition);
        }
        else 
            throw SyntaxError("Invalid type in type cast", peek().position);
    }

    return factor;
}

std::unique_ptr<Expression> Parser::parseArrayExpr() {
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

std::unique_ptr<Expression> Parser::parseSubject() {
    if (auto exp = parseIdOrFunCall())
        return exp;
    if (auto exp = parseLiterals())
        return exp;
    if (auto exp = parseNestedExpr())
        return exp;
      
    throw SyntaxError("Invalid expression", peek().position);
} 

std::unique_ptr<Expression> Parser::parseIdOrFunCall() {
    if (!match(TokenType::IDENTIFIER_T))
        return nullptr;

    std::string idName = std::get<std::string>(previous().value);
    Position idPosition = previous().position;

    if (auto exp = parseFunCall(idName, idPosition))
        return exp;

    return std::make_unique<Identifier>(idName, idPosition);
} 

std::unique_ptr<Expression> Parser::parseFunCall(std::string name, Position position) {
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

std::unique_ptr<Expression> Parser::parseLiterals() {
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

std::unique_ptr<Expression> Parser::parseArrayLiteral() {
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

    // TODO: zamienić na template z Varargs (...)
    // Args&&...
    // + requires(std::constructible_from<Node>, Args..., Position>)
    return std::make_unique<ArrayLit>(std::move(values), arrPos);
}

std::unique_ptr<Expression> Parser::parseNestedExpr() {
    if (!match(TokenType::L_BRACKET_T))
        return nullptr;
    
    auto expr = parseExpression();

    if (!match(TokenType::R_BRACKET_T))
        error("Missing closing parenthesis", peek().position);

    return expr;
}
