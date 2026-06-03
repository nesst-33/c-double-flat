#include "ASTPrinter.h"
#include "Node.h"
#include <sstream>
#include <string>
#include <iostream>


void ASTPrinter::printType(BaseType type) {
    switch (type) {
        case BaseType::INT:
            result += "int";
            return;
        case BaseType::FLP:
            result += "flp";
            return;
        case BaseType::STR:
            result += "str";
            return;
        case BaseType::BOOL:
            result += "bool";
            return;
        case BaseType::VOID:
            result += "void";
            return;
    }
}

void ASTPrinter::printTypeInfo(TypeInfo typeInfo) {
    if (typeInfo.isConst)
        result += "const ";

    for (int i{}; i < typeInfo.arrayDepth; i++) {
        result += "arr ";
    }

    printType(typeInfo.type);
    result += " ";
}

void ASTPrinter::printParam(Parameter param) {
    printTypeInfo(param.type);
    result += param.name;
}

void ASTPrinter::visit(const IntLit& node) {
    result += std::to_string(node.getValue());
}

std::string escapeString(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '\n': result += "\\n"; break;
            case '\t': result += "\\t"; break;
            case '\r': result += "\\r";  break;
            case '\\': result += "\\\\"; break;
            case '"':  result += "\\\""; break;
            case '\'': result += "\\'";  break;
            default:   result += c;      break;
        }
    }
    return result;
}

void ASTPrinter::visit(const StrLit& node) {
    result += "\"" + escapeString(node.getValue()) + "\"";
}

void ASTPrinter::visit(const FlpLit& node) {
    result += std::to_string(node.getValue());
}

void ASTPrinter::visit(const BoolLit& node) {
    if (node.getValue())
        result += "true";
    else 
        result += "false";
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

void ASTPrinter::visit(const VarDeclStmt& node) {
    printTypeInfo(node.getType()); 
    result += node.getName();
    const auto& initializer = node.getInitializer();
    if (initializer) {
        result += " = ";
        initializer->accept(*this);
    }
}

void ASTPrinter::visit(const Scope& node) {
    printIndent();
    result += "{\n";
    indentLevel++;
    for (const auto& stmt : node.getStatements()) {
        if (!dynamic_cast<Scope*>(stmt.get())) {
            printIndent();
        }
        stmt->accept(*this);
        result += "\n";
    }
    indentLevel--;
    printIndent();
    result += "}";
}

void ASTPrinter::visit(const IfStmt& node) {
    result += "if (";
    node.getCondition()->accept(*this);
    result += ")\n";

    node.getScope()->accept(*this);
    if (node.getElse()) {
        result += "\n";
        printIndent();
        result += "else\n";
        node.getElse()->accept(*this);
    }
}

void ASTPrinter::visit(const WhileStmt& node) {
    result += "while (";
    node.getCondition()->accept(*this);
    result += ")\n";
    node.getBody()->accept(*this);
}

void ASTPrinter::visit(const FuncDeclStmt& node) {
    printTypeInfo(node.getTypeInfo());
    result += node.getName();
    result += "(";
    const auto& params = node.getParams();
    for (size_t i{}; i < params.size(); i++) {
        printParam(params[i]);
        if (i < params.size() - 1)
            result += ", ";
    }
    result += ")\n";
    node.getBody()->accept(*this);
}

void ASTPrinter::visit(const FunCallStmt& node) {
    node.getFunCall()->accept(*this);
}

void ASTPrinter::visit(const Program& node) {
    for (const auto& statement : node.getStatements()) {
        statement->accept(*this);
        result += "\n";
    }
}

