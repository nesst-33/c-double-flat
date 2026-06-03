#ifndef _AST_LIST_H
#define _AST_LIST_H

// List of classes helpful for X-Macros (see Visitor.h)
#define AST_NODE_LIST(V) \
    V(Program) \
    V(AsExpr) \
    V(AndExpr) \
    V(OrExpr) \
    V(AddExpr) \
    V(SubExpr) \
    V(DivExpr) \
    V(MultExpr) \
    V(ModExpr) \
    V(ConcatExpr) \
    V(SplitExpr) \
    V(ConjunExpr) \
    V(AppendExpr) \
    V(ExtractExpr) \
    V(EqExpr) \
    V(NotEqExpr) \
    V(GreatExpr) \
    V(LessExpr) \
    V(GreatEqExpr) \
    V(LessEqExpr) \
    V(ArrayExpr) \
    V(PositiveExpr) \
    V(NegativeExpr) \
    V(NotExpr) \
    V(CardinalityExpr) \
    V(IntLit) \
    V(StrLit) \
    V(FlpLit) \
    V(BoolLit) \
    V(ArrayLit) \
    V(FunCall) \
    V(Identifier) \
    V(FunCallStmt) \
    V(IfStmt) \
    V(WhileStmt) \
    V(Scope) \
    V(RetStmt) \
    V(VarDeclStmt) \
    V(FuncDeclStmt) \
    V(BasicAssignStmt) \
    V(AddAssignStmt) \
    V(SubAssignStmt) \
    V(MultAssignStmt) \
    V(DivAssignStmt) \
    V(ModAssignStmt) \
    V(ConcatAssignStmt) \

#endif
