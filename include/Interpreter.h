#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include "Visitor.h"
#include "Value.h"
#include "Environment.h"
#include "PrintFunction.h"
#include <memory>
#include <utility>


class Interpreter : public Visitor {
public:
    Interpreter() : m_globals(std::make_shared<Environment>()) {
        Value printFun = Value(std::make_shared<PrintFunction>());
        m_globals->defineFunction({BaseType::VOID}, "print", std::move(printFun));
        m_env = m_globals;
        
    }

    void executeScope(const std::vector<std::unique_ptr<Statement>>& statements, std::shared_ptr<Environment> env);
    std::shared_ptr<Environment> getGlobals() const { return m_globals; }

    bool isReturning() const { return m_isReturning; }
    void setReturning(bool ret) { m_isReturning = ret; }
    Value getReturnValue() const { return returnValue; }

#define VISIT_DECL(T) void visit(const T& node) override;
AST_NODE_LIST(VISIT_DECL)
#undef VISIT_DECL

    std::shared_ptr<Environment> m_env;
private:
    Value lastResult;
    bool m_isReturning{}; 
    Value returnValue;

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
    
    const std::shared_ptr<Environment> m_globals;
};

#endif
