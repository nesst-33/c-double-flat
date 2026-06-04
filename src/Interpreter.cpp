#include "Interpreter.h"
#include "Node.h"
#include <stdexcept>

// ENTRYPOINT
void Interpreter::visit(const Program& node) {
    for (const auto& statement : node.getStatements())
        statement->accept(*this);
}

// LITERALS
// I know the code below violates DRY, but I want it to stay readable (I could use macros here)
void Interpreter::visit(const StrLit& node) {
    lastResult = Value(node.getValue());
}

void Interpreter::visit(const BoolLit& node) {
    lastResult = Value(node.getValue());
}

void Interpreter::visit(const IntLit& node) {
    lastResult = Value(node.getValue());
}

void Interpreter::visit(const FlpLit& node) {
    lastResult = Value(node.getValue());
}

void Interpreter::visit(const ArrayLit& node) {
    const auto& arrElements = node.getValues();
    auto arrVal = std::make_shared<Value::ArrayType>();

    if (arrElements.empty()) {
        lastResult = Value(std::move(arrVal));
        return;
    }

    for (const auto& element : arrElements) {
        element->accept(*this);
        arrVal->push_back(lastResult);
    }

    lastResult = Value(std::move(arrVal));
}

// EXPRESSIONS
// Numeric
void Interpreter::visit(const AddExpr& node) {
    node.getLeftFactor()->accept(*this);
    Value leftVal = lastResult;
    node.getRightFactor()->accept(*this);
    Value rightVal = lastResult;

    lastResult = leftVal + rightVal;
}

void Interpreter::visit(const SubExpr& node) {
    node.getLeftFactor()->accept(*this);
    Value leftVal = lastResult;
    node.getRightFactor()->accept(*this);
    Value rightVal = lastResult;

    lastResult = leftVal - rightVal;
}

void Interpreter::visit(const MultExpr& node) {
    node.getLeftFactor()->accept(*this);
    Value leftVal = lastResult;
    node.getRightFactor()->accept(*this);
    Value rightVal = lastResult;

    lastResult = leftVal * rightVal;
}

void Interpreter::visit(const DivExpr& node) {
    node.getLeftFactor()->accept(*this);
    Value leftVal = lastResult;
    node.getRightFactor()->accept(*this);
    Value rightVal = lastResult;

    lastResult = leftVal / rightVal;
}

void Interpreter::visit(const ModExpr& node) {
    node.getLeftFactor()->accept(*this);
    Value leftVal = lastResult;
    node.getRightFactor()->accept(*this);
    Value rightVal = lastResult;

    lastResult = leftVal % rightVal;
}

void Interpreter::visit(const AsExpr& node) {
}

void Interpreter::visit(const CardinalityExpr& node) {
}

void Interpreter::visit(const PositiveExpr& node) {
}

void Interpreter::visit(const NegativeExpr& node) {
}

void Interpreter::visit(const NotExpr& node) {
}

void Interpreter::visit(const ConcatExpr& node) {
}

void Interpreter::visit(const ConjunExpr& node) {
}

void Interpreter::visit(const SplitExpr& node) {
}

void Interpreter::visit(const AppendExpr& node) {
}

void Interpreter::visit(const ExtractExpr& node) {
}

void Interpreter::visit(const LessExpr& node) {
}

void Interpreter::visit(const GreatExpr& node) {
}

void Interpreter::visit(const LessEqExpr& node) {
}

void Interpreter::visit(const GreatEqExpr& node) {
}

void Interpreter::visit(const EqExpr& node) {
}

void Interpreter::visit(const NotEqExpr& node) {
}

void Interpreter::visit(const AndExpr& node) {
}

void Interpreter::visit(const OrExpr& node) {
}

void Interpreter::visit(const ArrayExpr& node) {
}

// STATEMENTS
void Interpreter::visit(const VarDeclStmt& node) {
    Value val;

    // TODO: array declaration 
    if (const auto& initializer = node.getInitializer()) {
        initializer->accept(*this);
        val = lastResult.castValue(node.getType().type);
    }
    else {
        switch(node.getType().type) {
            case BaseType::INT:
                val.setValue(0);
                break;
            case BaseType::FLP:
                val.setValue(0.);
                break;
            case BaseType::BOOL:
                val.setValue(false);
                break;
            case BaseType::STR:
                val.setValue("");
                break;
            case BaseType::VOID:
                throw std::runtime_error("Cannot declare void variables");
        }
    }
    m_env.define(node.getType(), node.getName(), std::move(val));
}


void Interpreter::visit(const Identifier& node) {
}

void Interpreter::visit(const FunCall& node) {
}


void Interpreter::visit(const BasicAssignStmt& node) {
}

void Interpreter::visit(const AddAssignStmt& node) {
}

void Interpreter::visit(const SubAssignStmt& node) {
}

void Interpreter::visit(const MultAssignStmt& node) {
}

void Interpreter::visit(const DivAssignStmt& node) {
}

void Interpreter::visit(const ModAssignStmt& node) {
}

void Interpreter::visit(const ConcatAssignStmt& node) {
}

void Interpreter::visit(const RetStmt& node) {
}

void Interpreter::visit(const Scope& node) {
}

void Interpreter::visit(const IfStmt& node) {
}

void Interpreter::visit(const WhileStmt& node) {
}

void Interpreter::visit(const FuncDeclStmt& node) {
}

void Interpreter::visit(const FunCallStmt& node) {
}


