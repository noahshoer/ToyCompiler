#include "AST/Expr.hpp"
#include "AST/ASTVisitor.hpp"

void NumberExpr::accept(ASTVisitor &visitor) {
    visitor.visitNumberExpr(*this);
}

void VariableExpr::accept(ASTVisitor &visitor) {
    visitor.visitVariableExpr(*this);
}

void BinaryExpr::accept(ASTVisitor &visitor) {
    visitor.visitBinaryExpr(*this);
}

void CallExpr::accept(ASTVisitor &visitor) {
    visitor.visitCallExpr(*this);
}