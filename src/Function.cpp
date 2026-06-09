#include "Function.h"

Value Function::call(Interpreter& interpreter,
        const std::vector<Value>& arguments) const { 
    auto funcEnv = std::make_shared<Environment>(interpreter.getGlobals());
    TypeInfo expectedType = m_declaration.getTypeInfo();

    const auto& params = m_declaration.getParams();
    for (size_t i{}; i < params.size(); i++)
        funcEnv->assignIdentifier(params[i].name, arguments[i]);

    interpreter.executeScope(m_declaration.getBody(), funcEnv);
    if (interpreter.isReturning()) {
        interpreter.setReturning(false);
        Value retValue = std::move(interpreter.getReturnValue());
        return retValue.matchType(expectedType);
    }

    if (expectedType.type != BaseType::VOID) {
        throw std::runtime_error(std::format("Function '{}' was supposed to return a value of type {} but returned none",
                    m_declaration.getName(), expectedType.toString()));
    }

    return Value();
}
