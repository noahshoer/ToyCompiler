#include "AST/Expr.hpp"
#include "AST/ASTVisitor.hpp"
#include "AST/ValueVisitor.hpp"

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

llvm::Value* NumberExpr::accept(ValueVisitor &visitor) {
    return  visitor.visitNumberExpr(*this);
}

llvm::Value* VariableExpr::accept(ValueVisitor &visitor) {
    return visitor.visitVariableExpr(*this);
}

llvm::Value* BinaryExpr::accept(ValueVisitor &visitor) {
    return visitor.visitBinaryExpr(*this);
}

llvm::Value* CallExpr::accept(ValueVisitor &visitor) {
    return visitor.visitCallExpr(*this);
}