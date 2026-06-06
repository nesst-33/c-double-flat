#include "Interpreter.h"
#include "Node.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <format>


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
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = std::move(leftVal + rightVal);
}

void Interpreter::visit(const SubExpr& node) {
    // TODO: add array handling
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = std::move(leftVal - rightVal);
}

void Interpreter::visit(const MultExpr& node) {
    // TODO: add array handling
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = std::move(leftVal * rightVal);
}

void Interpreter::visit(const DivExpr& node) {
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = std::move(leftVal / rightVal);
}

void Interpreter::visit(const ModExpr& node) {
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = std::move(leftVal % rightVal);
}

void Interpreter::visit(const AsExpr& node) {
    node.getCastedExpr()->accept(*this);
    lastResult = std::move(lastResult.castValue(node.getType()));
}

void Interpreter::visit(const PositiveExpr& node) {
    node.getFactor()->accept(*this);
}

void Interpreter::visit(const NegativeExpr& node) {
    node.getFactor()->accept(*this);
    lastResult = std::move(lastResult.negateNum());
}

// Array + string operators
void Interpreter::visit(const CardinalityExpr& node) {
    node.getFactor()->accept(*this);
    lastResult = std::move(lastResult.getCardinality());
}

// Indexes have to be of type int
// Only arrays or strings can be indexed
// Numbers will be implicitly casted to strings
void Interpreter::visit(const ArrayExpr& node) {
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = leftVal[rightVal];
}

// You can only concatenate strings and arrays
// Mixed types are disallowed
// Concatenation creates a shallow copy (for performance reasons)
void Interpreter::visit(const ConcatExpr& node) {
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = leftVal.concatenate(rightVal);
}

void Interpreter::visit(const ConjunExpr& node) {
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = leftVal.intersection(rightVal);
}

void Interpreter::visit(const SplitExpr& node) {
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = leftVal.split(rightVal);
}

// Appending happens by shallow copy
void Interpreter::visit(const AppendExpr& node) {
    // TODO: finish
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
}

void Interpreter::visit(const ExtractExpr& node) {
    // TODO: implement
}

// Relational

// Inequality operators
//
// You cannot compare booleans or arrays (it makes no logical sense)
// For strings, classic string comparison will be used
// Numbers will be implicitly casted (also true for strings that represent numbers)
void Interpreter::visit(const LessExpr& node) {
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = leftVal < rightVal;
}

void Interpreter::visit(const GreatExpr& node) {
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = leftVal > rightVal;
}

void Interpreter::visit(const LessEqExpr& node) {
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = leftVal <= rightVal;
}

void Interpreter::visit(const GreatEqExpr& node) {
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = leftVal >= rightVal;
}

// Equality operators
//
// All of the rules above are true, plus:
// - Arrays will be (deeply) compared element by element 
// - If comparing against a boolean, the other value will be implicitly casted to bool
void Interpreter::visit(const EqExpr& node) {
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = leftVal == rightVal;
}

void Interpreter::visit(const NotEqExpr& node) {
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = leftVal != rightVal;
}

// Logical
void Interpreter::visit(const NotExpr& node) {
    node.getFactor()->accept(*this);
    lastResult = std::move(lastResult.logicalNot());
}

void Interpreter::visit(const AndExpr& node) {
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);   
    lastResult = std::move(leftVal.logicalAnd(rightVal));
}

void Interpreter::visit(const OrExpr& node) {
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);   
    lastResult = std::move(leftVal.logicalOr(rightVal));
}

// Variable call
void Interpreter::visit(const Identifier& node) {
    lastResult = m_env.get(node.getName());
}

// Function call
void Interpreter::visit(const FunCall& node) {
}


// STATEMENTS
void Interpreter::visit(const VarDeclStmt& node) {
    Value val;
    if ( const auto& init = node.getInitializer() ) {
        init->accept(*this);
        val = lastResult;
    }
    m_env.define(node.getType(), node.getName(), val);
}

void Interpreter::visit(const BasicAssignStmt& node) {
    Expression* lhs = node.getLhs().get(); 
    const auto& rhs = node.getRhs();
    rhs->accept(*this);
    Value assignedVal = lastResult;

    if (auto* idNode = dynamic_cast<Identifier*>(lhs)) {
        m_env.assign(idNode->getName(), std::move(assignedVal)); 
    }
    else if (auto* indexNode = dynamic_cast<ArrayExpr*>(lhs)) {
        ArrayExpr* leftFactor = indexNode;
        ArrayExpr* parentNode = leftFactor;
        while ((leftFactor = dynamic_cast<ArrayExpr*>(leftFactor->getLeftFactor().get()))) {
            parentNode = leftFactor;
        }

        Identifier* idNode = dynamic_cast<Identifier*>(parentNode->getLeftFactor().get());
        if (!idNode) {
            // THIS SHOULD NEVER RUN (technically the parser shouldn't allow it to happen)
            throw std::runtime_error("Assigned array has no identifier");
        }

        const std::string& name = idNode->getName();
        TypeInfo arrTypeInfo = m_env.getTypeInfo(name);

        if (arrTypeInfo.isConst) 
            throw std::runtime_error("Variable '" + name + "' is immutable");
        
        auto [leftVal, rightVal] = evaluateBinaryFactors(*indexNode); 

        int targetDepth = leftVal.getDepth() - 1;
        int depth = assignedVal.getDepth();
        if ( depth != targetDepth ) {
            throw std::runtime_error(std::format("Array should be nested {} time(s); is nested {} time(s)",
                targetDepth, depth));
        }

        auto arrPtr = std::get<std::shared_ptr<Value::ArrayType>>(leftVal.getValue());
        int idx = rightVal.getIndex();

        try {
            arrPtr->at(idx) = assignedVal.castValue(arrTypeInfo.type);
        } catch (const std::out_of_range& e) {
            throw std::runtime_error("Index is out of range");
        }
    }
    else
        throw std::runtime_error("Invalid assignment target");
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


