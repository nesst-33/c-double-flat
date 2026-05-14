#ifndef _NODE_H
#define _NODE_H

#include <algorithm>
#include <utility>
#include <vector>
#include <memory>
#include "Token.h"

class Visitor;


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

// BASE NODE CLASS
class Node {
public:
    virtual ~Node() = default;
    virtual void accept(Visitor& v) = 0;
};

// EXPRESSIONS
class Expression : public Node {
public:
    virtual ~Expression() = default;
};

class AsExpr : public Expression {
public:
    AsExpr(std::unique_ptr<Expression> castedExpr,
            BaseType type,
            Position asPos)
    : m_castedExpr(std::move(castedExpr)), m_type(type), m_asPos(asPos) {}
    void accept(Visitor& v) override;
private:
    std::unique_ptr<Expression> m_castedExpr;
    BaseType m_type;
    Position m_asPos;
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
    void accept(Visitor &v) override; 
};

class OrExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
};

class AddExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
};

class SubExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
};

class DivExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
};

class MultExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
};

class ModExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
};

class ConcatExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
};

class SplitExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
};

class ConjunExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
};

class AppendExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
};

class ExtractExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
};

class EqExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
};

class NotEqExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
};

class GreatExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
};

class LessExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
};

class GreatEqExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
};

class LessEqExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
};

// Either indexing or predicate filtering
class ArrayExpr : public BinaryExpr {
public:
    using BinaryExpr::BinaryExpr;
    void accept(Visitor &v) override; 
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
    void accept(Visitor &v) override; 
};

class NegativeExpr : public UnaryExpr {
public:
    using UnaryExpr::UnaryExpr;
    void accept(Visitor &v) override; 
};

class NotExpr : public UnaryExpr {
public:
    using UnaryExpr::UnaryExpr;
    void accept(Visitor &v) override; 
};

class CardinalityExpr : public UnaryExpr {
public:
    using UnaryExpr::UnaryExpr;
    void accept(Visitor &v) override; 
};

// LITERALS
class IntLit : public Expression {
public:
    IntLit(int value, Position position) : m_value(value), m_position(position) {}
    void accept(Visitor &v) override; 
private:
    int m_value;
    Position m_position;
};

class StrLit : public Expression {
public:
    StrLit(std::string value, Position position) : m_value(value), m_position(position) {}
    void accept(Visitor &v) override; 
private:
    std::string m_value;
    Position m_position;
};

class FlpLit : public Expression {
public:
    FlpLit(double value, Position position) : m_value(value), m_position(position) {}
    void accept(Visitor &v) override; 
private:
    double m_value;
    Position m_position;
};

class BoolLit : public Expression {
public:
    BoolLit(bool value, Position position) : m_value(value), m_position(position) {}
    void accept(Visitor &v) override; 
private:
    bool m_value;
    Position m_position;
};

class ArrayLit : public Expression {
public:
    ArrayLit(std::vector<std::unique_ptr<Expression>> values, Position position)
        : m_values(std::move(values)), m_position(position) {}
    void accept(Visitor &v) override; 
private:
    std::vector<std::unique_ptr<Expression>> m_values;
    Position m_position;
};

// FUNCTION CALLS, ARRAY CALL, IDENTIFIERS AND OTHERS
class FunCall : public Expression {
public:
    FunCall(std::string name, std::vector<std::unique_ptr<Expression>> arguments, Position position)
        : m_name(name), m_arguments(std::move(arguments)), m_position(position) {}
    void accept(Visitor &v) override; 
private:
    std::string m_name;
    std::vector<std::unique_ptr<Expression>> m_arguments;
    Position m_position;
};

class Identifier : public Expression {
public:
    Identifier(std::string name, Position position) : m_name(name), m_position(position) {}
    void accept(Visitor &v) override; 
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
    void accept(Visitor &v) override; 
private:
    std::vector<std::unique_ptr<Statement>> m_statements{};
};

class FunCallStmt : public Statement {
public:
    FunCallStmt(std::unique_ptr<Expression> funCall, Position pos)
        : m_funCall(std::move(funCall)), Statement(pos) {}
    void accept(Visitor &v) override; 
private:
    std::unique_ptr<Expression> m_funCall;
};

class IfStmt : public Statement {
public:
    IfStmt(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> scope,
            std::unique_ptr<Statement> elseStmt, Position position)
        : m_condition(std::move(condition)), m_scope(std::move(scope))
        , m_else(std::move(elseStmt)), Statement(position) {}
    void accept(Visitor &v) override; 
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
    void accept(Visitor &v) override; 
private:
    std::unique_ptr<Expression> m_condition;
    std::unique_ptr<Statement> m_body;
    Position m_position;
};

class Scope : public Statement {
public:
    Scope(std::vector<std::unique_ptr<Statement>> statements, Position pos)
        : Statement(pos), m_statements(std::move(statements)) {}
    void accept(Visitor &v) override; 
private:
    std::vector<std::unique_ptr<Statement>> m_statements;
};

class RetStmt : public Statement {
public:
    RetStmt(std::unique_ptr<Expression> expr, Position pos)
        : m_expression(std::move(expr)), Statement(pos) {}
    void accept(Visitor &v) override; 
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
    void accept(Visitor &v) override; 

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
    void accept(Visitor &v) override; 
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
    void accept(Visitor &v) override; 
};

class AddAssignStmt : public AssignStmt {
public:
    using AssignStmt::AssignStmt;
    void accept(Visitor &v) override; 
};

class SubAssignStmt : public AssignStmt { 
public:
    using AssignStmt::AssignStmt;
    void accept(Visitor &v) override; 
};

class MultAssignStmt : public AssignStmt { 
public:
    using AssignStmt::AssignStmt;
    void accept(Visitor &v) override; 
};

class DivAssignStmt : public AssignStmt { 
public:
    using AssignStmt::AssignStmt;
    void accept(Visitor &v) override; 
};

class ModAssignStmt : public AssignStmt { 
public:
    using AssignStmt::AssignStmt;
    void accept(Visitor &v) override; 
};

class ConcatAssignStmt : public AssignStmt { 
public:
    using AssignStmt::AssignStmt;
    void accept(Visitor &v) override; 
};

#endif
