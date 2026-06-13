#ifndef _ICALLABLE_H
#define _ICALLABLE_H

#include <ostream>
#include <variant>
#include <vector>
#include <string>

class Value;
class Interpreter;

class ICallable {
public:
    virtual int arity() const = 0;
    virtual Value call(Interpreter& interpreter, 
            const std::vector<std::variant<Value, std::string>>& arguments) const = 0;
    virtual std::string toString() const = 0;
    virtual ~ICallable() = default;

    friend std::ostream& operator<<(std::ostream& os, const ICallable& callable) {
        os << callable.toString();
        return os;
    }
};

#endif
