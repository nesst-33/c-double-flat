#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include "Visitor.h"
#include "Value.h"
#include "Environment.h"


class Interpreter : public Visitor {
public:
    Interpreter() : m_env(Environment()) {}

#define VISIT_DECL(T) void visit(const T& node) override;
AST_NODE_LIST(VISIT_DECL)
#undef VISIT_DECL

    Environment m_env;
private:
    Value lastResult;
};

#endif
