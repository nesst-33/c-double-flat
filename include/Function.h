#ifndef _FUNCTION_H
#define _FUNCTION_H

#include "ICallable.h"
#include "Node.h"
#include "Value.h"
#include "Interpreter.h"

class Function : public ICallable {
public:
    Function(const FuncDeclStmt& declaration) : m_declaration(declaration) {}
    int arity() const override {
        return m_declaration.getParams().size();
    }

    Value call(Interpreter& interpreter,
            const std::vector<std::variant<Value, RefInfo>>& arguments) const override;

    std::string toString() const override {
        return "<" + m_declaration.getName() + " fn>";
    }
private:
    const FuncDeclStmt& m_declaration;
};

#endif
