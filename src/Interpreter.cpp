#include "Interpreter.h"
#include "Node.h"
#include "Function.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <format>
#include <variant>


// ENTRYPOINT
void Interpreter::visit(const Program& node) {
    for (const auto& statement : node.getStatements()) {
        statement->accept(*this);
        if (m_isReturning)
            break;
    }
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
    // if (auto* funCallPtr = dynamic_cast<FunCall*>(node.getRightFactor().get())) { 
    // }
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
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = leftVal.append(rightVal);
}

void Interpreter::visit(const ExtractExpr& node) {
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = leftVal.extract(rightVal);
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
    lastResult = m_env->get(node.getName());
}

// Function call
void Interpreter::visit(const FunCall& node) {
    std::string funcName = node.getName();
    Value::Type funcVariant = m_env->get(funcName).getValue();

    auto* functionPtr = std::get_if<std::shared_ptr<ICallable>>(&funcVariant);
    if (!functionPtr)
        throw std::runtime_error("Variable '" + funcName + "' is not callable");

    std::shared_ptr<ICallable> function = *functionPtr;

    if (node.getArguments().size() != function->arity()) {
        throw std::runtime_error(std::format("Expected {} arguments but got {}",
                    function->arity(), node.getArguments().size()));
    }

    // Argument can either be an r-value or variable name
    std::vector<std::variant<Value, std::string>> arguments{};
    arguments.reserve(node.getArguments().size());

    for (const auto& argument: node.getArguments()) {
        if (auto idNode = dynamic_cast<Identifier*>(argument.get())) {
            arguments.emplace_back(idNode->getName());
        } else {
            argument->accept(*this);
            arguments.push_back(std::move(lastResult));
        }
    }

    lastResult = function->call(*this, arguments);
}


// STATEMENTS
void Interpreter::visit(const VarDeclStmt& node) {
    Value val;
    if ( const auto& init = node.getInitializer() ) {
        init->accept(*this);
        val = lastResult;
    }
    m_env->define(node.getType(), node.getName(), val);
}

Identifier* Interpreter::getIdNodePtr(ArrayExpr* arrIndexExprPtr) {
    ArrayExpr* leftFactor{arrIndexExprPtr};
    ArrayExpr* parentNode{arrIndexExprPtr};
    while ((leftFactor = dynamic_cast<ArrayExpr*>(leftFactor->getLeftFactor().get())))
        parentNode = leftFactor;

    Identifier* idNode = dynamic_cast<Identifier*>(parentNode->getLeftFactor().get());
    return idNode;
}

void Interpreter::executeAssignment(Expression* lhs, Value assignedVal) {
    if (auto* idNode = dynamic_cast<Identifier*>(lhs)) {
        m_env->assignIdentifier(idNode->getName(), std::move(assignedVal)); 
    }
    else if (auto* indexNode = dynamic_cast<ArrayExpr*>(lhs)) {
        auto [leftVal, rightVal] = evaluateBinaryFactors(*indexNode); 
        Identifier* idNode = getIdNodePtr(indexNode);
        m_env->assignArrayOrStr(leftVal, rightVal, assignedVal, idNode);
    }
    else
        throw std::runtime_error("Invalid assignment target");
    
}

void Interpreter::visit(const BasicAssignStmt& node) {
    Expression* lhs = node.getLhs().get(); 
    const auto& rhs = node.getRhs();
    rhs->accept(*this);
    Value assignedVal = std::move(lastResult);

    executeAssignment(lhs, assignedVal);
}

template <typename NodeType>
void Interpreter::executeOpAssignment(const NodeType& node, assignmentOp op) {
    Expression* lhs = node.getLhs().get();

    lhs->accept(*this);
    Value currentVal = std::move(lastResult);

    node.getRhs()->accept(*this);
    Value modifierVal = std::move(lastResult);

    Value computedVal = op(currentVal, modifierVal);
    executeAssignment(lhs, std::move(computedVal));
}

void Interpreter::visit(const AddAssignStmt& node) {
    executeOpAssignment(node, [](const Value& l, const Value& r) { return l + r; });
}

void Interpreter::visit(const SubAssignStmt& node) {
    executeOpAssignment(node, [](const Value& l, const Value& r) { return l - r; });
}

void Interpreter::visit(const MultAssignStmt& node) {
    executeOpAssignment(node, [](const Value& l, const Value& r) { return l * r; });
}

void Interpreter::visit(const DivAssignStmt& node) {
    executeOpAssignment(node, [](const Value& l, const Value& r) { return l / r; });
}

void Interpreter::visit(const ModAssignStmt& node) {
    executeOpAssignment(node, [](const Value& l, const Value& r) { return l % r; });
}

void Interpreter::visit(const ConcatAssignStmt& node) {
    executeOpAssignment(node, [](const Value& l, const Value& r) { return l.concatenate(r); });
}

void Interpreter::executeScope(const std::vector<std::unique_ptr<Statement>>& statements,
        std::shared_ptr<Environment> env) {
    // RAII guard that will automatically switch back the environment if
    // an exception is thrown
    struct EnvGuard {
        std::shared_ptr<Environment>& current;
        std::shared_ptr<Environment> old;
        ~EnvGuard() { current = old; }
    } guard{ this->m_env, this->m_env };

    this->m_env = env;

    for (const auto& statement : statements) {
        statement->accept(*this);
        if (m_isReturning)
            break;
    }
}

void Interpreter::visit(const Scope& node) {
    executeScope(node.getStatements(), std::make_shared<Environment>(this->m_env));
}

bool isTruthy(const Value& val) {
    return std::get<bool>(val.castValue(BaseType::BOOL).getValue());
}

bool Interpreter::checkCondition(const auto& condition) {
    condition->accept(*this);
    return isTruthy(lastResult);
}

void Interpreter::visit(const IfStmt& node) {
    const auto& elseBlock = node.getElse();
    const auto& condition = node.getCondition();

    if (checkCondition(condition))
        node.getScope()->accept(*this);
    else if (elseBlock) 
        elseBlock->accept(*this);
}

void Interpreter::visit(const WhileStmt& node) {
    const auto& condition = node.getCondition();
    while (checkCondition(condition)) {
        node.getBody()->accept(*this);     
        if (m_isReturning)
            break;
    }
}

void Interpreter::visit(const RetStmt& node) {
    const auto& retExpr = node.getExpr();
    if (retExpr) {
        retExpr->accept(*this);
        returnValue = std::move(lastResult);
    } else
        returnValue = Value();

    m_isReturning = true;
}

void Interpreter::visit(const FuncDeclStmt& node) {
    std::shared_ptr<ICallable> funcPtr = std::make_shared<Function>(node);
    m_globals->defineFunction(node.getTypeInfo(), node.getName(), Value(std::move(funcPtr)));
}

void Interpreter::visit(const FunCallStmt& node) {
    node.getFunCall()->accept(*this); 
}


