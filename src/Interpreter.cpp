#include "Interpreter.h"
#include "Node.h"

// I know the code below violates DRY, but I want it to stay readable (I could use templates or macros here)
void Interpreter::visit(const StrLit& node) {
    lastResult = Value(node.getValue());
}

void Interpreter::visit(const BoolLit& node) {
    lastResult = Value(node.getValue());
}

void Interpreter::visit(const IntLit& node) {
    lastResult = Value(node.getValue());
}

void Interpreter::visit(const FlpLit& node) {
    lastResult = Value(node.getValue());
}

void Interpreter::visit(const AddExpr& node) {
    node.getLeftFactor()->accept(*this);
    Value leftVal = lastResult;

    node.getRightFactor()->accept(*this);
    Value rightVal = lastResult;


}
