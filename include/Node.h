#include <vector>
#include <memory>

class Node {
};

// EXPRESSIONS
class Expression : public Node {
public:
    virtual ~Expression() = default;
};

// BINARY EXPRESSIONS
class BinaryExpr : public Expression {
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

class AsExpr : public BinaryExpr {
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

// UNARY EXPRESSIONS
class UnaryExpr: public Expression {
    UnaryExpr(std::unique_ptr<Expression> factor, Position operatorPos)
        : m_factor(std::move(factor)), m_operatorPos(operatorPos) {} 
private:
    std::unique_ptr<Expression> m_factor;
    Position m_operatorPos;
};



class Statement : public Node {};

class Scope : public Statement {};
class ElseStmt : public Node {};

class Program : public Node {
public:
    Program(std::vector<std::unique_ptr<Statement>> statements) 
        : m_statements(std::move(statements)) {}
private:
    std::vector<std::unique_ptr<Statement>> m_statements{};
};


class IfStmt : public Statement {
public:
    IfStmt(std::unique_ptr<Expression> expr, std::unique_ptr<Scope> scope,
            std::unique_ptr<ElseStmt> elseStmt)
        : m_expression(std::move(expr)), m_scope(std::move(scope))
        , m_else(std::move(elseStmt)) {}
private:
    std::unique_ptr<Expression> m_expression;
    std::unique_ptr<Scope> m_scope;
    std::unique_ptr<ElseStmt> m_else;
};

class WhileStmt : public Statement {};
class VarOrFuncDecl : public Statement {};
class VoidFuncDecl : public Statement {};
class RetStmt : public Statement {};
class IdArrFuncCall : public Statement {};
