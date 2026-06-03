#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include "Visitor.h"
#include "Value.h"

class Environment {};

class Interpreter : public Visitor {
public:
    Interpreter(Environment& env) : m_env(env) {}

#define VISIT_DECL(T) void visit(const T& node) override;
AST_NODE_LIST(VISIT_DECL)
#undef VISIT_DECL

private:
    Value lastResult;
    Environment& m_env;
};

#endif
