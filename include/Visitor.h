#ifndef _VISITOR_H
#define _VISITOR_H

#include "ASTList.h"

// Macro for quick forward declarations using the #define from ASTList.h
#define FORWARD_DECLARE(T) class T;
AST_NODE_LIST(FORWARD_DECLARE)

class Visitor {
public:
    virtual ~Visitor() = default;

#define VISIT_METHOD(T) virtual void visit(const T& node) = 0;
AST_NODE_LIST(VISIT_METHOD)
#undef VISIT_METHOD // we don't want a local macro to leak out, so I undefine it

};

#endif
