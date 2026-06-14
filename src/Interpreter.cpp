#include "Interpreter.h"
#include "Node.h"
#include "Function.h"
#include <memory>
#include <stdexcept>
#include <format>
#include <variant>


// ENTRYPOINT
void Interpreter::visit(const Program& node) {
    try {
        for (const auto& statement : node.getStatements()) {
            execute(statement);
            if (m_isReturning)
                break;
        }
    } catch (const std::runtime_error& e) {
        reportError(e.what(), {});
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
        evaluate(element);
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
    auto [leftVal, rightVal] = evaluateBinaryFactors(node);
    lastResult = std::move(leftVal - rightVal);
}

void Interpreter::visit(const MultExpr& node) {
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
    evaluate(node.getCastedExpr());
    lastResult = std::move(lastResult.castValue(node.getType()));
}

void Interpreter::visit(const PositiveExpr& node) {
    evaluate(node.getFactor());
}

void Interpreter::visit(const NegativeExpr& node) {
    evaluate(node.getFactor());
    lastResult = std::move(lastResult.negateNum());
}

// Array + string operators
void Interpreter::visit(const CardinalityExpr& node) {
    evaluate(node.getFactor());
    lastResult = std::move(lastResult.getCardinality());
}

Value Interpreter::applyMapOperation(const Value& arrayVal, FunCall* funCall, int placeholderIdx) {
    auto valVariant = arrayVal.getValue();
    auto* arrPtr = std::get_if<std::shared_ptr<Value::ArrayType>>(&valVariant); 
    if (!arrPtr)
        reportError("Cannot map functions to non-array types", funCall->getPosition());

    auto arr = *arrPtr;
    Value funcWrapper = m_env->get(funCall->getName());
    auto callable = std::get<std::shared_ptr<ICallable>>(funcWrapper.getValue());

    // Pre-evaluate function arguments + put empty Value instead of placeholder
    std::vector<std::variant<Value, RefInfo>> evaluatedArgs(funCall->getArguments().size());
    for (size_t i{}; i < funCall->getArguments().size(); i++) {
        if (i == placeholderIdx)
            evaluatedArgs[i] = Value();
        else {
            evaluate(funCall->getArguments()[i]);
            evaluatedArgs[i] = std::move(lastResult);
        }
    }

    Value::ArrayType result;
    result.reserve(arr->size());

    for (const Value& item : *arr) {
        evaluatedArgs[placeholderIdx] = item;
        Value returnVal = callable->call(*this, evaluatedArgs);
        result.push_back(returnVal);
    }

    return Value(std::make_shared<Value::ArrayType>(result));
}

// Indexes have to be of type int
// Only arrays or strings can be indexed
// Numbers will be implicitly casted to strings
void Interpreter::visit(const ArrayExpr& node) {
    evaluate(node.getLeftFactor());
    Value leftVal = std::move(lastResult);

    if (auto* funCallPtr = dynamic_cast<FunCall*>(node.getRightFactor().get())) { 
        const auto& arguments = funCallPtr->getArguments();
        for (size_t i{}; i < arguments.size(); i++) {
            if (auto* idPtr = dynamic_cast<Identifier*>(arguments[i].get())) {
                if (idPtr->getName() == "_") {
                    lastResult = applyMapOperation(leftVal, funCallPtr, i);
                    return;
                }
            } 
        }
    }

    evaluate(node.getRightFactor());
    Value rightVal = std::move(lastResult);
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
    evaluate(node.getFactor());
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
        reportError("Variable '" + funcName + "' is not callable", node.getPosition());

    std::shared_ptr<ICallable> function = *functionPtr;

    if (node.getArguments().size() != function->arity()) {
        reportError(std::format("Expected {} arguments but got {}",
                    function->arity(), node.getArguments().size()), node.getPosition());
    }

    // Argument can either be an r-value or variable name
    std::vector<std::variant<Value, RefInfo>> arguments{};
    arguments.reserve(node.getArguments().size());

    for (const auto& argument: node.getArguments()) {
        if (auto idNode = dynamic_cast<Identifier*>(argument.get())) {
            arguments.emplace_back(m_env->getRefInfo(idNode->getName()));
        } else {
            evaluate(argument);
            arguments.push_back(std::move(lastResult));
        }
    }

    lastResult = function->call(*this, arguments);
}


// STATEMENTS
void Interpreter::visit(const VarDeclStmt& node) {
    Value val;
    if ( const auto& init = node.getInitializer() ) {
        evaluate(init);
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
        reportError("Invalid assignment target", lhs->getPosition());
}

void Interpreter::visit(const BasicAssignStmt& node) {
    Expression* lhs = node.getLhs().get(); 
    const auto& rhs = node.getRhs();
    evaluate(rhs);
    Value assignedVal = std::move(lastResult);

    executeAssignment(lhs, assignedVal);
}

template <typename NodeType>
void Interpreter::executeOpAssignment(const NodeType& node, assignmentOp op) {
    evaluate(node.getLhs());
    Value currentVal = std::move(lastResult);

    evaluate(node.getRhs());
    Value modifierVal = std::move(lastResult);

    Expression* lhs = node.getLhs().get();
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
        execute(statement);
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
    evaluate(condition);
    return isTruthy(lastResult);
}

void Interpreter::visit(const IfStmt& node) {
    const auto& elseBlock = node.getElse();
    const auto& condition = node.getCondition();

    if (checkCondition(condition))
        execute(node.getScope());
    else if (elseBlock) 
        execute(elseBlock);
}

void Interpreter::visit(const WhileStmt& node) {
    const auto& condition = node.getCondition();
    while (checkCondition(condition)) {
        execute(node.getBody());
        if (m_isReturning)
            break;
    }
}

void Interpreter::visit(const RetStmt& node) {
    const auto& retExpr = node.getExpr();
    if (retExpr) {
        evaluate(retExpr);
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
    evaluate(node.getFunCall());
}

void Interpreter::reportError(const std::string& message, Position pos) {
    m_err.report(std::make_unique<RuntimeError>(message, Severity::ERROR, pos));
}

template <typename NodeType>
std::pair<Value, Value> Interpreter::evaluateBinaryFactors(const NodeType& node) {
    evaluate(node.getLeftFactor());
    Value leftVal = std::move(lastResult);
    evaluate(node.getRightFactor());
    Value rightVal = std::move(lastResult);
    return {std::move(leftVal), std::move(rightVal)};
}
