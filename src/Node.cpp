#include "Node.h"
#include "Visitor.h"

#define IMPLEMENT_ACCEPT(T) void T::accept(Visitor& v) { v.visit(*this); }

AST_NODE_LIST(IMPLEMENT_ACCEPT)
