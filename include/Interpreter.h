#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include "Visitor.h"
#include "Value.h"
#include "Environment.h"
#include <utility>


class Interpreter : public Visitor {
public:
    Interpreter() : m_env(Environment()) {}

#define VISIT_DECL(T) void visit(const T& node) override;
AST_NODE_LIST(VISIT_DECL)
#undef VISIT_DECL

    Environment m_env;
private:
    Value lastResult;

    template <typename NodeType>
    std::pair<Value, Value> evaluateBinaryFactors(const NodeType& node) {
        node.getLeftFactor()->accept(*this);
        Value leftVal = lastResult;
        node.getRightFactor()->accept(*this);
        Value rightVal = lastResult;
        return {std::move(leftVal), std::move(rightVal)};
    }
};

#endif
