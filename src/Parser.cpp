#include "Parser.h"
#include "Token.h"
#include <memory>
#include <vector>

auto Parser::createBinOpTable() 
    -> std::array<BinOpFactory, std::to_underlying(TokenType::UNKNOWN)> {

    std::array<BinOpFactory, std::to_underlying(TokenType::UNKNOWN)> table{};

    table[std::to_underlying(TokenType::PLUS_T)] = makeBinOpFactory<AddExpr>(); 
    table[std::to_underlying(TokenType::MINUS_T)] = makeBinOpFactory<SubExpr>(); 
    table[std::to_underlying(TokenType::MULT_T)] = makeBinOpFactory<MultExpr>(); 
    table[std::to_underlying(TokenType::DIV_T)] = makeBinOpFactory<DivExpr>(); 
    table[std::to_underlying(TokenType::MOD_T)] = makeBinOpFactory<ModExpr>(); 
    table[std::to_underlying(TokenType::CONCAT_T)] = makeBinOpFactory<ConcatExpr>(); 
    table[std::to_underlying(TokenType::CONJUN_T)] = makeBinOpFactory<ConjunExpr>(); 
    table[std::to_underlying(TokenType::SPLIT_T)] = makeBinOpFactory<SplitExpr>(); 
    table[std::to_underlying(TokenType::APPEND_T)] = makeBinOpFactory<AppendExpr>(); 
    table[std::to_underlying(TokenType::EXTRACT_T)] = makeBinOpFactory<ExtractExpr>(); 
    table[std::to_underlying(TokenType::GREATER_T)] = makeBinOpFactory<GreatExpr>(); 
    table[std::to_underlying(TokenType::LESSER_T)] = makeBinOpFactory<LessExpr>(); 
    table[std::to_underlying(TokenType::EQ_T)] = makeBinOpFactory<EqExpr>(); 
    table[std::to_underlying(TokenType::NOT_EQ_T)] = makeBinOpFactory<NotEqExpr>(); 
    table[std::to_underlying(TokenType::GREATER_EQ_T)] = makeBinOpFactory<GreatEqExpr>(); 
    table[std::to_underlying(TokenType::LESSER_EQ_T)] = makeBinOpFactory<LessEqExpr>(); 
    table[std::to_underlying(TokenType::AND_T)] = makeBinOpFactory<AndExpr>(); 
    table[std::to_underlying(TokenType::OR_T)] = makeBinOpFactory<OrExpr>(); 

    return table;
}

auto Parser::createUnaryOpTable() 
    -> std::array<UnaryOpFactory, std::to_underlying(TokenType::UNKNOWN)> {

    std::array<UnaryOpFactory, std::to_underlying(TokenType::UNKNOWN)> table{};

    table[std::to_underlying(TokenType::PLUS_T)] = makeUnaryOpFactory<PositiveExpr>();
    table[std::to_underlying(TokenType::MINUS_T)] = makeUnaryOpFactory<NegativeExpr>();
    table[std::to_underlying(TokenType::NOT_T)] = makeUnaryOpFactory<NotExpr>();
    table[std::to_underlying(TokenType::CARDINALITY_T)] = makeUnaryOpFactory<CardinalityExpr>();

    return table;
}

auto Parser::createAssignTable() 
    -> std::array<AssignFactory, std::to_underlying(TokenType::UNKNOWN)> {

    std::array<AssignFactory, std::to_underlying(TokenType::UNKNOWN)> table{}; 

    table[std::to_underlying(TokenType::ASSIGN_T)] = makeAssignFactory<BasicAssignStmt>();
    table[std::to_underlying(TokenType::ADD_ASSIGN_T)] = makeAssignFactory<AddAssignStmt>();
    table[std::to_underlying(TokenType::SUB_ASSIGN_T)] = makeAssignFactory<SubAssignStmt>();
    table[std::to_underlying(TokenType::MULT_ASSIGN_T)] = makeAssignFactory<MultAssignStmt>();
    table[std::to_underlying(TokenType::DIV_ASSIGN_T)] = makeAssignFactory<DivAssignStmt>();
    table[std::to_underlying(TokenType::MOD_ASSIGN_T)] = makeAssignFactory<ModAssignStmt>();
    table[std::to_underlying(TokenType::CONCAT_ASSIGN_T)] = makeAssignFactory<ConcatAssignStmt>();

    return table;
}

void Parser::synchronize() {
    while (!isAtEnd()) {
        advance();
        if (previous().type == TokenType::NEWLINE_T)
            return;

        // switch(peek().type) {
        //     case TokenType::IF_T:
        //     case TokenType::WHILE_T:
        //     case TokenType::L_BRACE_T:
        //     case TokenType::INT_T:
        //     case TokenType::FLP_T:
        //     case TokenType::STR_T:
        //     case TokenType::BOOL_T:
        //     case TokenType::VOID_T:
        //     case TokenType::ARR_T:
        //     case TokenType::IDENTIFIER_T:
        //     case TokenType::RETURN_T:
        //         return;
        //     default: break;
        // }

        //advance();
    }
}

// program = { [statement], newline }, EOT ;
Program Parser::parse() {
    std::vector<std::unique_ptr<Statement>> statements{};

    while (!isAtEnd()) {
        try {
            if (auto statement = parseStatement()) {
                statements.push_back(std::move(statement));
                consume(TokenType::NEWLINE_T, "Missing terminating newline");
            }
            else throwIfMissing(match(TokenType::NEWLINE_T), "Invalid statement");

        } catch (SyntaxError e) {
            synchronize();
        }
    }

    return Program(std::move(statements));
}

// statement = if_stmt | while_stmt | scope | var_or_func_decl
//           | void_func_decl | id_arr_func_call 
std::unique_ptr<Statement> Parser::parseStatement() {
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
    if (auto st = parseIdArrFunCall()) 
        return st;

    return nullptr;
}

// scope =  "{", { [scoped_stmt], newline }, "}"
std::unique_ptr<Statement> Parser::parseScope() {
    if (!match(TokenType::L_BRACE_T))
        return nullptr;

    Position scopePos = previous().position;

    std::vector<std::unique_ptr<Statement>> statements {};

    while (!match(TokenType::R_BRACE_T)) {
        try { 
            if (auto statement = parseScopedStmt()) {
                statements.push_back(std::move(statement));
                consume(TokenType::NEWLINE_T, "Missing terminating newline");
            }
            else throwIfMissing(match(TokenType::NEWLINE_T), "Invalid statement");
        } catch (SyntaxError e) {
            synchronize();
        }
    }

    return std::make_unique<Scope>(std::move(statements), scopePos);
}

// scoped_stmt = var_decl | scope_stmt | if_stmt | while_stmt | return_stmt
//             | id_arr_func_call
std::unique_ptr<Statement> Parser::parseScopedStmt() {

    if (auto st = parseIfStmt())
        return st;
    if (auto st = parseWhileStmt())
        return st;
    if (auto st = parseScope()) 
        return st;
    if (auto st = parseIdArrFunCall()) 
        return st;
    if (auto st = parseVarDecl())
        return st;
    if (auto st = parseRetStmt())
        return st;

    return nullptr;
}

// var_decl = type, identifier, ["=", expression]
std::unique_ptr<Statement> Parser::parseVarDecl() {
    Position startPos = peek().position;
    auto typeOpt = parseType();
    if (!typeOpt)
        return nullptr;

    TypeInfo type = *typeOpt;
    
    std::string name = consumeIdentifier();
    return parseVarDeclAssign(type, name, startPos);
}

// if_stmt = "if", condition, [newline], scope, [else_body];
std::unique_ptr<Statement> Parser::parseIfStmt() {
    if (!match(TokenType::IF_T))
        return nullptr;

    Position ifPos = previous().position;

    auto condition = parseCondition();

    match(TokenType::NEWLINE_T);

    auto scope = parseScope();
    throwIfMissing(scope, "Missing if body");

    auto elseStmt = parseElseBody();

    return std::make_unique<IfStmt>(std::move(condition), std::move(scope), std::move(elseStmt), ifPos);
}

// condition = "(", expression, ")"
std::unique_ptr<Expression> Parser::parseCondition() {
    consume(TokenType::L_BRACKET_T, "Missing left bracket");

    auto condition = parseExpression();
    throwIfMissing(condition, "Invalid condition");
    
    consume(TokenType::R_BRACKET_T, "Missing right bracket");

    return condition;
}

// else_body = "else", [newline], scope
std::unique_ptr<Statement> Parser::parseElseBody() {
    if (!match(TokenType::ELSE_T))
        return nullptr;

    match(TokenType::NEWLINE_T);

    auto scope = parseScope();
    throwIfMissing(scope, "Missing else body");

    return scope;
}

// while_stmt = "while", condition, [newline], scope
std::unique_ptr<Statement> Parser::parseWhileStmt() {
    if (!match(TokenType::WHILE_T))
        return nullptr;

    Position whilePos = previous().position;
    auto condition = parseCondition();
    match(TokenType::NEWLINE_T);
    auto whileBody = parseScope();
    throwIfMissing(whileBody, "Missing while body");

    return std::make_unique<WhileStmt>(std::move(condition), std::move(whileBody), whilePos);
}

// var_or_func_decl = type, identifier, (func_declaration | [var_decl_assign])
std::unique_ptr<Statement> Parser::parseVarOrFuncDecl() {
    Position startPos = peek().position;
    auto typeOpt = parseType();
    if (!typeOpt)
        return nullptr;

    TypeInfo type = *typeOpt;
    std::string name = consumeIdentifier();

    if (auto func = parseFuncDecl(type, name, startPos))
        return func;

    return parseVarDeclAssign(type, name, startPos);
}

// void_func_decl = "void", identifier, func_declaration
std::unique_ptr<Statement> Parser::parseVoidFuncDecl() {
    if (!match(TokenType::VOID_T))
        return nullptr;
    Position startPos = previous().position;
    TypeInfo type {BaseType::VOID};
    
    std::string name = consumeIdentifier();
    auto func = parseFuncDecl(type, name, startPos);
    throwIfMissing(func, "Missing function declaration");

    return func;
}

// func_declaration = "(", [parameters], ")", [newline], scope 
std::unique_ptr<Statement> Parser::parseFuncDecl(TypeInfo type, std::string name, Position pos) {
    if (!match(TokenType::L_BRACKET_T))
        return nullptr;

    std::vector<Parameter> params = parseParameters();
    consume(TokenType::R_BRACKET_T, "Missing closing bracket in parameter list");
    match(TokenType::NEWLINE_T);
    auto body = parseScope();
    throwIfMissing(body, "Missing function body");

    return std::make_unique<FuncDeclStmt>(type, name, std::move(params), std::move(body), pos);
}

// var_decl_assign = "=", expression
std::unique_ptr<Statement> Parser::parseVarDeclAssign(TypeInfo type, std::string name, Position pos) {
    std::unique_ptr<Expression> initializer;
    if (match(TokenType::ASSIGN_T)) {
        initializer = parseExpression();
        throwIfMissing(initializer, "Expected expression after assignment operator");
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
    std::string name = consumeIdentifier();
    params.push_back({std::move(type), name, previous().position});

    while (match(TokenType::COMMA_T)) {
        typeOpt = parseType();
        
        if (!typeOpt) {
            m_errorHandler.report(std::make_unique<SyntaxError>("Trailing comma in parameter list", Severity::WARNING, previous().position));
            continue;
        }

        type = *typeOpt; 
        std::string name = consumeIdentifier();
        Parameter param = {std::move(type), name, previous().position};
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
        if (type.isConst || type.arrayDepth > 0) {
            m_errorHandler.report(std::make_unique<SyntaxError>("Missing type in declaration", Severity::ERROR, peek().position));
            // throw SyntaxError("Missing type in declaration", peek().position);
        }
        return std::nullopt;
    }

    return type;
}

// return_stmt = "return", [expression]
std::unique_ptr<Statement> Parser::parseRetStmt() {
    if (!match(TokenType::RETURN_T))
        return nullptr;

    Position retPos = previous().position;
    auto expression = parseExpression();

    return std::make_unique<RetStmt>(std::move(expression), retPos);
}

// id_arr_func_call = identifier, (func_call | assign)
std::unique_ptr<Statement> Parser::parseIdArrFunCall() {
    if (!match(TokenType::IDENTIFIER_T))
        return nullptr;

    std::string name = std::get<std::string>(previous().value);
    Position pos = previous().position;

    if (auto funCall = parseFunCall(name, pos))
        return std::make_unique<FunCallStmt>(std::move(funCall), pos);

    return parseAssign(name, pos);
}

// assign = [array_idx], assign_op, expression
std::unique_ptr<Statement> Parser::parseAssign(std::string name, Position pos) {
    auto lhs = parseArrayIdx(name, pos);

    throwIfMissing(match(TokenType::ASSIGN_T, TokenType::ADD_ASSIGN_T, TokenType::SUB_ASSIGN_T,
                TokenType::MULT_ASSIGN_T, TokenType::DIV_ASSIGN_T, TokenType::MOD_ASSIGN_T,
                TokenType::CONCAT_ASSIGN_T), 
            "Expected assignment or function call after identifier");

    Position assPos = previous().position;
    TokenType assType = previous().type;
    auto rhs = parseExpression();
    throwIfMissing(rhs, "Expected expression after assignment operator");
    
    return assTypeToObject[std::to_underlying(assType)](std::move(lhs), assPos, std::move(rhs));
}

// array_idx = "[", expression, "]", {"[", expression, "]"}
std::unique_ptr<Expression> Parser::parseArrayIdx(std::string name, Position pos) {
    std::unique_ptr<Expression> arrayExpr = std::make_unique<Identifier>(name, pos);

    while (match(TokenType::L_SQUARE_T)) {
        Position squarePos = previous().position;

        if (match(TokenType::R_SQUARE_T))
            m_errorHandler.report(std::make_unique<SyntaxError>("Missing expression inside square brackets", Severity::ERROR, squarePos));

        auto indexExpr = parseExpression();

        // throwIfMissing(indexExpr, "Missing array index inside square brackets", previous().position);
        consume(TokenType::R_SQUARE_T, "Missing closing square bracket");
        arrayExpr = std::make_unique<ArrayExpr>(std::move(arrayExpr), squarePos, std::move(indexExpr));
    }
    
    return arrayExpr;
}

// expression = logical_and, {"or", logical_and}
std::unique_ptr<Expression> Parser::parseExpression() {
    auto leftFactor = parseAndExpr();
    if (!leftFactor)
        return nullptr; 

    while(match(TokenType::OR_T))
    {
        Position orPosition = previous().position;
        auto rightFactor = parseAndExpr();
        throwIfMissing(rightFactor, "Missing expression after 'or' keyword");

        leftFactor = binaryOpTypeToObject[std::to_underlying(TokenType::OR_T)](std::move(leftFactor), orPosition, std::move(rightFactor));
    }
    return leftFactor;
}

// logical_and = equality, {"and", equality}
std::unique_ptr<Expression> Parser::parseAndExpr() {
    auto leftFactor = parseEqualityExpr();
    if (!leftFactor)
        return nullptr;

    while(match(TokenType::AND_T)) {
        Position andPosition = previous().position;
        auto rightFactor = parseEqualityExpr();
        throwIfMissing(rightFactor, "Missing expression after 'and' keyword");

        leftFactor = binaryOpTypeToObject[std::to_underlying(TokenType::AND_T)](std::move(leftFactor), andPosition, std::move(rightFactor));
    }

    return leftFactor;
}

// equality = relational, [ ("==" | "!="), relational ]
std::unique_ptr<Expression> Parser::parseEqualityExpr() {
    auto leftFactor = parseRelationalExpr();
    if (!leftFactor)
        return nullptr;

    if (match(TokenType::EQ_T, TokenType::NOT_EQ_T)) {
        TokenType eqType = previous().type;
        Position eqPosition = previous().position;
        auto rightFactor = parseRelationalExpr();
        throwIfMissing(rightFactor, "Missing expression after equality operator");
        leftFactor = binaryOpTypeToObject[std::to_underlying(eqType)](std::move(leftFactor), eqPosition, std::move(rightFactor));
    }

    return leftFactor;
}

// relational = arr_ops, [ ("<" | ">" | "<=" | ">="), arr_ops]
std::unique_ptr<Expression> Parser::parseRelationalExpr() {
    auto leftFactor = parseArrayOpsExpr();
    if (!leftFactor)
        return nullptr;

    if (match(TokenType::LESSER_T, TokenType::LESSER_EQ_T, TokenType::GREATER_T, TokenType::GREATER_EQ_T)) {
        TokenType relType = previous().type;
        Position eqPosition = previous().position;
        auto rightFactor = parseArrayOpsExpr();
        throwIfMissing(rightFactor, "Missing expression after inequality operator");
        leftFactor = binaryOpTypeToObject[std::to_underlying(relType)](std::move(leftFactor), eqPosition, std::move(rightFactor));
    }

    return leftFactor;
}

// arr_ops = additive, { ("~" | "&" | ":" | "<<" | ">>"), additive }
std::unique_ptr<Expression> Parser::parseArrayOpsExpr() {
    auto leftFactor = parseAdditiveExpr();
    if(!leftFactor)
        return nullptr;

    while (match(TokenType::CONCAT_T, TokenType::CONJUN_T, TokenType::SPLIT_T, TokenType::APPEND_T, TokenType::EXTRACT_T)) {
        TokenType arrOpType = previous().type;
        Position arrOpPosition = previous().position;
        auto rightFactor = parseAdditiveExpr();
        throwIfMissing(rightFactor, "Missing expression after array operator");
        leftFactor = binaryOpTypeToObject[std::to_underlying(arrOpType)](std::move(leftFactor), arrOpPosition, std::move(rightFactor));
    }

    return leftFactor;
}

// additive = multipl, { ("+" | "-"), multipl }
std::unique_ptr<Expression> Parser::parseAdditiveExpr() {
    auto leftFactor = parseMultiplExpr();
    if (!leftFactor)
        return nullptr;
    
    while (match(TokenType::PLUS_T, TokenType::MINUS_T)) {
        TokenType addType = previous().type;
        Position addPosition = previous().position;
        auto rightFactor = parseMultiplExpr();
        throwIfMissing(rightFactor, "Missing expression after additive operator");
        leftFactor = binaryOpTypeToObject[std::to_underlying(addType)](std::move(leftFactor), addPosition, std::move(rightFactor));
    }

    return leftFactor;
}

// multipl = unary, { ("*" | "/" | "%" ), unary }
std::unique_ptr<Expression> Parser::parseMultiplExpr() {
    auto leftFactor = parseUnaryExpr();
    if (!leftFactor)
        return nullptr;

    while (match(TokenType::MULT_T, TokenType::DIV_T, TokenType::MOD_T)) {
        TokenType multType = previous().type;
        Position multPosition = previous().position;
        auto rightFactor = parseUnaryExpr();
        throwIfMissing(rightFactor, "Missing expression after multiplicative operator");
        leftFactor = binaryOpTypeToObject[std::to_underlying(multType)](std::move(leftFactor), multPosition, std::move(rightFactor));
    }

    return leftFactor;
}

// unary = {"+" | "-" | "not"}, postfix
std::unique_ptr<Expression> Parser::parseUnaryExpr() {
    if (match(TokenType::PLUS_T, TokenType::MINUS_T, TokenType::NOT_T)) {
        TokenType unaryType = previous().type;
        Position unaryPosition = previous().position;
        auto factor = parseUnaryExpr();
        throwIfMissing(factor, "Expected expression after unary operator", unaryPosition);
        return unaryOpTypeToObject[std::to_underlying(unaryType)](std::move(factor), unaryPosition); 
    }
    
    return parsePostfix();
}

// postfix = type_cast, {"!" | ("as", ("int" | "flp" | "bool" | "str"))}
std::unique_ptr<Expression> Parser::parsePostfix() {
    auto factor = parseArrayExpr();
    if (!factor)
        return nullptr;

    while (match(TokenType::CARDINALITY_T, TokenType::AS_T)) {
        if (previous().type == TokenType::CARDINALITY_T)
            factor = unaryOpTypeToObject[std::to_underlying(TokenType::CARDINALITY_T)](std::move(factor), previous().position);
        else {
            Position asPosition = previous().position;

            BaseType type{};
            if (match(TokenType::STR_T))
                type = BaseType::STR;
            else if (match(TokenType::INT_T))
                type = BaseType::INT;
            else if (match(TokenType::BOOL_T))
                type = BaseType::BOOL;
            else if (match(TokenType::FLP_T))
                type = BaseType::FLP;
            else {
                m_errorHandler.report(std::make_unique<SyntaxError>(
                            "Invalid/missing type in type cast", 
                            Severity::ERROR, 
                            peek().position));
            }
            factor = std::make_unique<AsExpr>(std::move(factor), type, asPosition);
        }
    }

    return factor;
}

// type_cast = arr_expr, {"as", ("int" | "flp" | "bool" | "str")}
std::unique_ptr<Expression> Parser::parseTypeCast() {
    auto factor = parseArrayExpr();
    if (!factor)
        return nullptr;

    while (match(TokenType::AS_T)) {
        Position asPosition = previous().position;

        BaseType type{};
        if (match(TokenType::STR_T))
            type = BaseType::STR;
        else if (match(TokenType::INT_T))
            type = BaseType::INT;
        else if (match(TokenType::BOOL_T))
            type = BaseType::BOOL;
        else if (match(TokenType::FLP_T))
            type = BaseType::FLP;
        else {
            m_errorHandler.report(std::make_unique<SyntaxError>(
                        "Invalid type in type cast", 
                        Severity::ERROR, 
                        peek().position));
        }
        // throw SyntaxError("Invalid type in type cast", peek().position);

        factor = std::make_unique<AsExpr>(std::move(factor), type, asPosition);
    }

    return factor;
}

// arr_expr = subject, {"[", expression, "]"}
std::unique_ptr<Expression> Parser::parseArrayExpr() {
    auto arrObj = parseSubject();
    if (!arrObj)
        return nullptr;


    while (match(TokenType::L_SQUARE_T)) {
        Position squarePos = previous().position;

        if (match(TokenType::R_SQUARE_T))
            m_errorHandler.report(std::make_unique<SyntaxError>("Missing expression inside square brackets", Severity::ERROR, squarePos));

        auto indexExpr = parseExpression();
        // throwIfMissing(indexExpr, "Missing or invalid array index/predicate inside square brackets");
        arrObj = std::make_unique<ArrayExpr>(std::move(arrObj), squarePos, std::move(indexExpr));
        consume(TokenType::R_SQUARE_T, "Missing closing square bracket");
    }

    return arrObj;
}

// subject = id_or_func_call | literal | nested_expr
std::unique_ptr<Expression> Parser::parseSubject() {
    if (auto exp = parseIdOrFunCall())
        return exp;
    if (auto exp = parseLiterals())
        return exp;
    if (auto exp = parseNestedExpr())
        return exp;
      
    // m_errorHandler.report(std::make_unique<SyntaxError>("Invalid expression", Severity::ERROR, peek().position));
    return nullptr;
    // throw SyntaxError("Invalid expression", peek().position);
} 

// id_or_func_call = identifier, [func_call]
std::unique_ptr<Expression> Parser::parseIdOrFunCall() {
    if (!match(TokenType::IDENTIFIER_T))
        return nullptr;

    std::string idName = std::get<std::string>(previous().value);
    Position idPosition = previous().position;

    if (auto exp = parseFunCall(idName, idPosition))
        return exp;

    return std::make_unique<Identifier>(idName, idPosition);
} 

// func_call = "(", [ arguments ], ")"
std::unique_ptr<Expression> Parser::parseFunCall(std::string name, Position position) {
    if (!match(TokenType::L_BRACKET_T))
        return nullptr;
    
    std::vector<std::unique_ptr<Expression>> arguments = parseArguments();
    consume(TokenType::R_BRACKET_T, "Missing closing bracket");

    return std::make_unique<FunCall>(name, std::move(arguments), position);
}

// arguments = expression, {",", expression}
std::vector<std::unique_ptr<Expression>> Parser::parseArguments() {
    std::vector<std::unique_ptr<Expression>> arguments {};
    auto argument = parseExpression();
    if (!argument)
        return arguments;

    arguments.push_back(std::move(argument));
    while (match(TokenType::COMMA_T)) {
        Position argPos = peek().position;
        argument = parseExpression();
        throwIfMissing(argument, "Invalid argument", argPos);
        arguments.push_back(std::move(argument));
    }

    return arguments;
}

// literal = int_lit | flp_lit | str_lit | bool_lit | arr_lit
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

// arr_lit = "[", [ expression, {",", expression} ], "}"
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
            throwIfMissing(value, "Invalid expression in array literal", valPos);
            values.push_back(std::move(value));
        }
    }

    consume(TokenType::R_SQUARE_T, "Missing closing square bracket in array literal");

    // TODO: zamienić na template z Varargs (...)
    // Args&&...
    // + requires(std::constructible_from<Node>, Args..., Position>)
    return std::make_unique<ArrayLit>(std::move(values), arrPos);
}

// nested_expr = "(", expression, ")"
std::unique_ptr<Expression> Parser::parseNestedExpr() {
    if (!match(TokenType::L_BRACKET_T))
        return nullptr;
    
    auto expr = parseExpression();
    consume(TokenType::R_BRACKET_T, "Missing closing parenthesis");

    return expr;
}
