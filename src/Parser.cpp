#include "Parser.h"

std::unique_ptr<Program> Parser::parse() {
    std::vector<std::unique_ptr<Statement>> statements{};
    while(!isAtEnd())
        if (auto statement = parseStatement())
            statements.push_back(std::move(statement));

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


std::unique_ptr<Statement> Parser::parseStatement() {
    
    // Ignore lines that are pure newline
    while (match(TokenType::NEWLINE_T));

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

std::unique_ptr<Statement> Parser::parseScopedStmt() {
    while(match(TokenType::NEWLINE_T));

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

std::unique_ptr<Statement> Parser::parseVarDecl() {
    Position startPos = peek().position;
    auto typeOpt = parseType();
    if (!typeOpt)
        return nullptr;
    TypeInfo type = *typeOpt;
    if (!match(TokenType::IDENTIFIER_T))
        throw SyntaxError("Expected identifier name after type", peek().position);

    std::string name = std::get<std::string>(previous().value);
    
    return parseVarDeclTail(type, name, startPos);
}

std::unique_ptr<Statement> Parser::parseIfStmt() {
    if (!match(TokenType::IF_T))
        return nullptr;

    Position ifPos = previous().position;

    auto condition = parseCondition();

    match(TokenType::NEWLINE_T);

    auto scope = parseScope();
    if (!scope) 
        throw SyntaxError("Ill-formed scope", peek().position);

    auto elseBody = parseElseBody();

    if (!elseBody) {
        if (match(TokenType::NEWLINE_T)) 
            elseBody = parseElseBody();
    } else 
        error("Expected 'else' or newline after if-statement scope", peek().position);

    return std::make_unique<IfStmt>(std::move(condition), std::move(scope), std::move(elseBody), ifPos);
}

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

std::unique_ptr<Statement> Parser::parseVarOrFuncDecl() {
    Position startPos = peek().position;
    auto typeOpt = parseType();
    if (!typeOpt)
        return nullptr;

    TypeInfo type = *typeOpt;

    if (!match(TokenType::IDENTIFIER_T))
        throw SyntaxError("Expected identifier name after type", peek().position);

    std::string name = std::get<std::string>(previous().value);
    if (auto func = parseFuncDecl(type, name, startPos))
        return func;
    
    if (!match(TokenType::NEWLINE_T))
        error("Missing terminating newline", peek().position);

    return parseVarDeclTail(type, name, startPos);
}

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

    // Czy tym nie powinien się zajmować interpreter (Environment)?
    if (auto search = functionMap.find(name); search == functionMap.end())
        functionMap.insert({name, pos});
    else
        throw SyntaxError("Function redefinition", pos);

    return std::make_unique<FuncDeclStmt>(type, name, std::move(params), std::move(body), pos);
}

std::unique_ptr<Statement> Parser::parseVarDeclTail(TypeInfo type, std::string name, Position pos) {
    std::unique_ptr<Expression> initializer;
    if (match(TokenType::ASSIGN_T)) {
        initializer = parseExpression();
        if (!initializer)
            throw SyntaxError("Invalid expression after '='", peek().position);
    }

    return std::make_unique<VarDeclStmt>(type, name, std::move(initializer), pos);
}

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

std::unique_ptr<Statement> Parser::parseRetStmt() {
    if (!match(TokenType::RETURN_T))
        return nullptr;

    Position retPos = previous().position;
    auto expression = parseExpression();

    if (!match(TokenType::NEWLINE_T))
        error("Missing terminating newline", peek().position);

    return std::make_unique<RetStmt>(std::move(expression), retPos);
}

std::unique_ptr<Statement> Parser::parseIdArrFunCall() {
    if (!match(TokenType::IDENTIFIER_T))
        return nullptr;

    std::string name = std::get<std::string>(previous().value);
    Position pos = previous().position;

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

// Uogólnić do parseBinaryOp()??
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

