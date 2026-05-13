#include <utility>
#include <vector>
#include <memory>
#include "Token.h"

class Node {};

// Type info for declarations
enum class BaseType { INT, FLP, STR, BOOL, VOID };

struct TypeInfo {
    BaseType type;
    bool isConst {};
    int arrayDepth {};
};

struct Parameter {
    TypeInfo type;
    std::string name;
    Position pos;
};

// EXPRESSIONS
class Expression : public Node {
public:
    virtual ~Expression() = default;
};

// BINARY EXPRESSIONS
class BinaryExpr : public Expression {
public:
    BinaryExpr(std::unique_ptr<Expression> leftFactor, 
            Position operatorPos, 
            std::unique_ptr<Expression> rightFactor)
        : m_leftFactor(std::move(leftFactor))
        , m_operatorPos(operatorPos)
        , m_rightFactor(std::move(rightFactor)) {}
private:
    std::unique_ptr<Expression> m_leftFactor;
    Position m_operatorPos;
    std::unique_ptr<Expression> m_rightFactor;
};

class AndExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

class OrExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

class AddExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

class SubExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

class DivExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

class MultExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

class ModExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

class ConcatExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

class SplitExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

class ConjunExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

class AppendExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

class ExtractExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

class EqExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

class NotEqExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

class GreatExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

class LessExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

class GreatEqExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

class LessEqExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};

// Either indexing or predicate filtering
class ArrayExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
};


// UNARY EXPRESSIONS
class UnaryExpr: public Expression {
public:
    UnaryExpr(std::unique_ptr<Expression> factor, Position operatorPos)
        : m_factor(std::move(factor)), m_operatorPos(operatorPos) {} 
private:
    std::unique_ptr<Expression> m_factor;
    Position m_operatorPos;
};

class PositiveExpr : public UnaryExpr {
public:
    using UnaryExpr::UnaryExpr;
};

class NegativeExpr : public UnaryExpr {
public:
    using UnaryExpr::UnaryExpr;
};

class NotExpr : public UnaryExpr {
public:
    using UnaryExpr::UnaryExpr;
};

class CardinalityExpr : public UnaryExpr {
public:
    using UnaryExpr::UnaryExpr;
};

// Type cast is a unary operator at its core (it completely changes behaviour depending on type)
class AsExpr : public UnaryExpr {
public:
    using UnaryExpr::UnaryExpr;
};

class StrCast : public AsExpr {
public:
    using AsExpr::AsExpr;
};

class IntCast : public AsExpr {
public:
    using AsExpr::AsExpr;
};

class BoolCast : public AsExpr {
public:
    using AsExpr::AsExpr;
};

class FlpCast : public AsExpr {
public:
    using AsExpr::AsExpr;
};

// LITERALS
class IntLit : public Expression {
public:
    IntLit(int value, Position position) : m_value(value), m_position(position) {}
private:
    int m_value;
    Position m_position;
};

class StrLit : public Expression {
public:
    StrLit(std::string value, Position position) : m_value(value), m_position(position) {}
private:
    std::string m_value;
    Position m_position;
};

class FlpLit : public Expression {
public:
    FlpLit(double value, Position position) : m_value(value), m_position(position) {}
private:
    double m_value;
    Position m_position;
};

class BoolLit : public Expression {
public:
    BoolLit(bool value, Position position) : m_value(value), m_position(position) {}
private:
    bool m_value;
    Position m_position;
};

class ArrayLit : public Expression {
public:
    ArrayLit(std::vector<std::unique_ptr<Expression>> values, Position position)
        : m_values(std::move(values)), m_position(position) {}
private:
    std::vector<std::unique_ptr<Expression>> m_values;
    Position m_position;
};

// FUNCTION CALLS, ARRAY CALL, IDENTIFIERS AND OTHERS
class FunCall : public Expression {
public:
    FunCall(std::string name, std::vector<std::unique_ptr<Expression>> arguments, Position position)
        : m_name(name), m_arguments(std::move(arguments)), m_position(position) {}
private:
    std::string m_name;
    std::vector<std::unique_ptr<Expression>> m_arguments;
    Position m_position;
};

class Identifier : public Expression {
public:
    Identifier(std::string name, Position position) : m_name(name), m_position(position) {}
private:
    std::string m_name;
    Position m_position;
};


class Statement : public Node {
public:
    Statement(Position pos) : m_position(pos) {}
private:
    Position m_position;
};


class Program : public Node {
public:
    Program(std::vector<std::unique_ptr<Statement>> statements) 
        : m_statements(std::move(statements)) {}
private:
    std::vector<std::unique_ptr<Statement>> m_statements{};
};

class FunCallStmt : public Statement {
public:
    FunCallStmt(std::unique_ptr<Expression> funCall, Position pos)
        : m_funCall(std::move(funCall)), Statement(pos) {}
private:
    std::unique_ptr<Expression> m_funCall;
};

class IfStmt : public Statement {
public:
    IfStmt(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> scope,
            std::unique_ptr<Statement> elseStmt, Position position)
        : m_condition(std::move(condition)), m_scope(std::move(scope))
        , m_else(std::move(elseStmt)), Statement(position) {}
private:
    std::unique_ptr<Expression> m_condition;
    std::unique_ptr<Statement> m_scope;
    std::unique_ptr<Statement> m_else;
    Position m_position;
};

class WhileStmt : public Statement {
public:
    WhileStmt(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> body, Position position)
        : m_condition(std::move(condition)), m_body(std::move(body)), Statement(position) {}
private:
    std::unique_ptr<Expression> m_condition;
    std::unique_ptr<Statement> m_body;
    Position m_position;
};

class Scope : public Statement {
public:
    Scope(std::vector<std::unique_ptr<Statement>> statements, Position pos)
        : Statement(pos), m_statements(std::move(statements)) {}
private:
    std::vector<std::unique_ptr<Statement>> m_statements;
};

class RetStmt : public Statement {
public:
    RetStmt(std::unique_ptr<Expression> expr, Position pos)
        : m_expression(std::move(expr)), Statement(pos) {}
private:
    std::unique_ptr<Expression> m_expression;
    Position m_position;
};

// DECLARATIONS
class VarDeclStmt : public Statement {
public:
    VarDeclStmt(TypeInfo type, 
            std::string name,
            std::unique_ptr<Expression> initializer,
            Position pos)
        : Statement(pos)
        , m_type(std::move(type))
        , m_name(std::move(name))
        , m_initializer(std::move(initializer)) {}

private:
    TypeInfo m_type;
    std::string m_name;
    std::unique_ptr<Expression> m_initializer;
};

class FuncDeclStmt : public Statement {
public:
    FuncDeclStmt(TypeInfo returnType,
            std::string name,
            std::vector<Parameter> params,
            std::unique_ptr<Statement> body,
            Position pos)
        : Statement(pos)
        , m_returnType(returnType)
        , m_name(std::move(name))
        , m_params(std::move(params))
        , m_body(std::move(body)) {}
private:
    TypeInfo m_returnType;
    std::string m_name;
    std::vector<Parameter> m_params;
    std::unique_ptr<Statement> m_body;
};

// ASSIGNMENTS
class AssignStmt : public Statement {
public:
    AssignStmt(std::unique_ptr<Expression> lhs, Position opPos, std::unique_ptr<Expression> rhs)
        : Statement(opPos), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {} 
private:
    std::unique_ptr<Expression> m_lhs; 
    std::unique_ptr<Expression> m_rhs; 
};

class BasicAssignStmt : public AssignStmt {
public:
    using AssignStmt::AssignStmt;
};

class AddAssignStmt : public AssignStmt {
public:
    using AssignStmt::AssignStmt;
};

class SubAssignStmt : public AssignStmt { 
public:
    using AssignStmt::AssignStmt;
};

class MultAssignStmt : public AssignStmt { 
public:
    using AssignStmt::AssignStmt;
};

class DivAssignStmt : public AssignStmt { 
public:
    using AssignStmt::AssignStmt;
};

class ModAssignStmt : public AssignStmt { 
public:
    using AssignStmt::AssignStmt;
};

class ConcatAssignStmt : public AssignStmt { 
public:
    using AssignStmt::AssignStmt;
};

