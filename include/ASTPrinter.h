#ifndef _AST_PRINTER_H
#define _AST_PRINTER_H

#include "Visitor.h"
#include <string>

#define BUF_ALLOC_SIZE 512

enum class BaseType;
struct TypeInfo;
struct Parameter;

class ASTPrinter : public Visitor {
public:
    ASTPrinter() { result.reserve(BUF_ALLOC_SIZE); }

    std::string getResult() const { return result; }

#define VISIT_DECL(T) void visit(const T& node) override;
AST_NODE_LIST(VISIT_DECL)
#undef VISIT_DECL

private:
    std::string result;
    int indentLevel{};

    void printIndent() { result += std::string(indentLevel, '\t'); }

    void printList(const auto& list) {
        for (size_t i{}; i<list.size(); i++) {
            list[i]->accept(*this);
            if (i<list.size() - 1)
                result += ", ";
        }
    }

    void printType(BaseType type); 
    void printTypeInfo(TypeInfo typeInfo);
    void printParam(Parameter param);

};

#endif
