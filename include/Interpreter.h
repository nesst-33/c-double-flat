#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include "Visitor.h"
#include "Value.h"
#include "Environment.h"
#include <memory>
#include <utility>


class Interpreter : public Visitor {
public:
    Interpreter() : m_env(std::make_shared<Environment>()) {}

#define VISIT_DECL(T) void visit(const T& node) override;
AST_NODE_LIST(VISIT_DECL)
#undef VISIT_DECL

    std::shared_ptr<Environment> m_env;
private:
    Value lastResult;

    template <typename NodeType>
    std::pair<Value, Value> evaluateBinaryFactors(const NodeType& node) {
        node.getLeftFactor()->accept(*this);
        Value leftVal = std::move(lastResult);
        node.getRightFactor()->accept(*this);
        Value rightVal = std::move(lastResult);
        return {std::move(leftVal), std::move(rightVal)};
    }

    Identifier* getIdNodePtr(ArrayExpr* arrIndexExprPtr);
    void executeAssignment(Expression* lhs, Value assignedVal);
    bool checkCondition(const auto& condition);

    using assignmentOp = Value (*)(const Value&, const Value&);
    template <typename NodeType>
    void executeOpAssignment(const NodeType& node, assignmentOp op);
    
};

#endif
