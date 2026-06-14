#ifndef _PRINT_H
#define _PRINT_H

#include "Environment.h"
#include "ICallable.h"
#include "Value.h"
#include <iostream>
#include <ostream>

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

class ClearFunction : public ICallable {
    int arity() const override { return 0; }
    std::string toString() const override { return "<clear fn>"; }

    Value call(Interpreter& interpreter,
            const std::vector<std::variant<Value, RefInfo>>& arguments) const override 
    {
            std::cout << "\033[2J\033[H" << std::flush; 
            return Value();
    }
};

#endif
