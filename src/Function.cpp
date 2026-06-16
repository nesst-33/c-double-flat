#include "Function.h"
#include <stdexcept>
#include <variant>

// template<class... Ts>
// struct overloaded : Ts... { using Ts::operator()...; };

Value Function::call(Interpreter& interpreter,
        const std::vector<std::variant<Value, RefInfo>>& arguments) const { 
    auto funcEnv = std::make_shared<Environment>(interpreter.getGlobals());
    TypeInfo expectedType = m_declaration.getTypeInfo();

    const auto& params = m_declaration.getParams();
 
    for (size_t i{}; i < params.size(); i++)
        std::visit(overloaded{
                [&](Value v) {
                    funcEnv->define(params[i].type, params[i].name, v);        
                },

                [&](const RefInfo& refInfo) {
                    funcEnv->defineReference(params[i].type, params[i].name, refInfo);
                }
            }, arguments[i]);

    auto* scope = dynamic_cast<Scope*>(m_declaration.getBody().get());
    interpreter.executeScope(scope->getStatements(), funcEnv);
    if (interpreter.isReturning()) {
        interpreter.setReturning(false);

        Value retValue = std::move(interpreter.getReturnValue());

        if (!std::holds_alternative<std::monostate>(retValue.getValue())) {
            if (expectedType.type == BaseType::VOID)
                throw std::runtime_error("Void function cannot return a value");
            else
                return retValue.matchType(expectedType);
        }
                
        return Value();
    }

    if (expectedType.type != BaseType::VOID) {
        throw std::runtime_error(std::format("Function '{}' was supposed to return a value of type {} but returned none",
                    m_declaration.getName(), expectedType.toString()));
    }

    return Value();
}
