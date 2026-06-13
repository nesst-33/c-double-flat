#include "Function.h"

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

Value Function::call(Interpreter& interpreter,
        const std::vector<std::variant<Value, std::string>>& arguments) const { 
    auto funcEnv = std::make_shared<Environment>(interpreter.getGlobals());
    TypeInfo expectedType = m_declaration.getTypeInfo();

    const auto& params = m_declaration.getParams();
 
    for (size_t i{}; i < params.size(); i++)
        std::visit(overloaded{
                [&](Value v) {
                    funcEnv->define(params[i].type, params[i].name, v);        
                },

                [&](const std::string& refName) {
                    funcEnv->defineReference(params[i].type, params[i].name, refName);
                }
            }, arguments[i]);

    auto* scope = dynamic_cast<Scope*>(m_declaration.getBody().get());
    interpreter.executeScope(scope->getStatements(), funcEnv);
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
