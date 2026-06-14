#ifndef _PRINT_H
#define _PRINT_H

#include "Environment.h"
#include "ICallable.h"
#include "Value.h"
#include <iostream>

class PrintFunction : public ICallable {
    int arity() const override { return 1; }
    std::string toString() const override { return "<print fn>"; }

    Value call(Interpreter& interpreter,
            const std::vector<std::variant<Value, RefInfo>>& arguments) 
        const override {

            std::visit(overloaded{
                [](const Value& v) { std::cout << v << "\n"; },
                [](const RefInfo& refInfo) {
                    std::cout << refInfo.value.get() << "\n";
                }

            }, arguments[0]);

            return Value();
    }
};

#endif
