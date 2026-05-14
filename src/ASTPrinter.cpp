#include "ASTPrinter.h"
#include "Node.h"
#include <string>


void ASTPrinter::printType(BaseType type) {
    switch (type) {
        case BaseType::INT:
            result += "int";
        case BaseType::FLP:
            result += "flp";
        case BaseType::STR:
            result += "str";
        case BaseType::BOOL:
            result += "bool";
        case BaseType::VOID:
            result += "bool";
    }
}

void ASTPrinter::visit(const IntLit& node) {
    result += std::to_string(node.getValue());
}

void ASTPrinter::visit(const StrLit& node) {
    result += node.getValue();
}

void ASTPrinter::visit(const FlpLit& node) {
    result += node.getValue();
}

void ASTPrinter::visit(const BoolLit& node) {
    result += node.getValue();
}

void ASTPrinter::visit(const ArrayLit& node) {
    result += "[";
    printList(node.getValues());
    result += "]";
}

void ASTPrinter::visit(const Identifier& node) {
    result += node.getName();
}

void ASTPrinter::visit(const FunCall& node) {
    result += node.getName() + "(";
    printList(node.getArguments());
    result += ")";
}

void ASTPrinter::visit(const AsExpr& node) {
    result += "(";
    node.getCastedExpr()->accept(*this);
    result += " as ";
    printType(node.getType()); 
    result += ")";
}

void ASTPrinter::visit(const CardinalityExpr& node) {
    result += "(";
    node.getFactor()->accept(*this);
    result += "!)";
}

void ASTPrinter::visit(const PositiveExpr& node) {
    result += "(+";
    node.getFactor()->accept(*this); 
    result += ")";
}

void ASTPrinter::visit(const NegativeExpr& node) {
    result += "(-";
    node.getFactor()->accept(*this); 
    result += ")";
}

void ASTPrinter::visit(const NotExpr& node) {
    result += "(not ";
    node.getFactor()->accept(*this); 
    result += ")";
}

void ASTPrinter::visit(const MultExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " * ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const DivExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " / ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const ModExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " % ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const AddExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " + ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const SubExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " - ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const ConcatExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " ~ ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const ConjunExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " & ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const SplitExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " : ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const AppendExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " << ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const ExtractExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " >> ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const LessExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " < ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const GreatExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " > ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const LessEqExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " <= ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const GreatEqExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " >= ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const EqExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " == ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const NotEqExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " != ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const AndExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " and ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const OrExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += " or ";
    node.getRightFactor()->accept(*this);
    result += ")";
}

void ASTPrinter::visit(const ArrayExpr& node) {
    result += "(";
    node.getLeftFactor()->accept(*this);
    result += "[";
    node.getRightFactor()->accept(*this);
    result += "])";
}

void ASTPrinter::visit(const BasicAssignStmt& node) {
    node.getLhs()->accept(*this);
    result += " = ";
    node.getRhs()->accept(*this);
}

void ASTPrinter::visit(const AddAssignStmt& node) {
    node.getLhs()->accept(*this);
    result += " += ";
    node.getRhs()->accept(*this);
}

void ASTPrinter::visit(const SubAssignStmt& node) {
    node.getLhs()->accept(*this);
    result += " -= ";
    node.getRhs()->accept(*this);
}

void ASTPrinter::visit(const MultAssignStmt& node) {
    node.getLhs()->accept(*this);
    result += " *= ";
    node.getRhs()->accept(*this);
}

void ASTPrinter::visit(const DivAssignStmt& node) {
    node.getLhs()->accept(*this);
    result += " /= ";
    node.getRhs()->accept(*this);
}

void ASTPrinter::visit(const ModAssignStmt& node) {
    node.getLhs()->accept(*this);
    result += " %= ";
    node.getRhs()->accept(*this);
}

void ASTPrinter::visit(const ConcatAssignStmt& node) {
    node.getLhs()->accept(*this);
    result += " ~= ";
    node.getRhs()->accept(*this);
}

void ASTPrinter::visit(const RetStmt& node) {
    result += "return ";
    node.getExpr()->accept(*this);
}

