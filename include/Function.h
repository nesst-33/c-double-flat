#ifndef _FUNCTION_H
#define _FUNCTION_H

#include "ICallable.h"
#include "Node.h"
#include "Value.h"
#include "Interpreter.h"
#include <algorithm>
#include <format>

class Function : public ICallable {
public:
    Function(const FuncDeclStmt& declaration) : m_declaration(declaration) {}
    int arity() const override {
        return m_declaration.getParams().size();
    }

    Value call(Interpreter& interpreter,
            const std::vector<Value>& arguments) const override;

    std::string toString() const override {
        return "<fn " + m_declaration.getName() + ">";
    }
private:
    const FuncDeclStmt& m_declaration;
};

#endif
