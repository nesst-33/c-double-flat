#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include "LangError.h"
#include "Token.h"
#include "Visitor.h"
#include "Value.h"
#include "Environment.h"
#include "PrintFunction.h"
#include "ErrorHandler.h"
#include <memory>
#include <utility>


class Interpreter : public Visitor {
public:
    Interpreter(ErrorHandler& errHandler) 
        : m_err(errHandler)
        , m_globals(std::make_shared<Environment>()) {
        Value printFun = Value(std::make_shared<PrintFunction>());
        m_globals->defineFunction({BaseType::VOID}, "print", std::move(printFun));

        Value clearFun = Value(std::make_shared<ClearFunction>());
        m_globals->defineFunction({BaseType::VOID}, "clear", std::move(clearFun));

        m_env = m_globals;
    }

    void interpret(const Program& program) {
        try {
            this->visit(program);
        } catch (const LangError& e) {}
    }

    void executeScope(const std::vector<std::unique_ptr<Statement>>& statements, std::shared_ptr<Environment> env);
    std::shared_ptr<Environment> getGlobals() const { return m_globals; }

    bool isReturning() const { return m_isReturning; }
    void setReturning(bool ret) { m_isReturning = ret; }
    Value getReturnValue() const { return returnValue; }


private:
#define VISIT_DECL(T) void visit(const T& node) override;
AST_NODE_LIST(VISIT_DECL)
#undef VISIT_DECL

    Value lastResult;
    bool m_isReturning{}; 
    Value returnValue;
    ErrorHandler& m_err;
    std::shared_ptr<Environment> m_env;
    const std::shared_ptr<Environment> m_globals;
    Position lastPosition;

    void execute(const std::unique_ptr<Statement>& statement) {
        lastPosition = statement->getPosition();
        statement->accept(*this);
    }
    void evaluate(const std::unique_ptr<Expression>& expression) {
        lastPosition = expression->getPosition();
        expression->accept(*this);
    }

    template <typename NodeType>
    std::pair<Value, Value> evaluateBinaryFactors(const NodeType& node); 

    Identifier* getIdNodePtr(ArrayExpr* arrIndexExprPtr);
    void executeAssignment(Expression* lhs, Value assignedVal);
    bool checkCondition(const auto& condition);
    Value applyMapOperation(const Value& arr, FunCall* funCall, int placeholderIdx);

    using assignmentOp = Value (*)(const Value&, const Value&);
    template <typename NodeType>
    void executeOpAssignment(const NodeType& node, assignmentOp op);

    void reportError(const std::string& message, Position pos);
};

#endif
