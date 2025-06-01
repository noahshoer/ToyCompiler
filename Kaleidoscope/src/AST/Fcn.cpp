#include "AST/ASTVisitor.hpp"
#include "AST/Fcn.hpp"
#include "AST/ValueVisitor.hpp"

void FcnPrototype::accept(ASTVisitor &visitor) {
    visitor.visitFcnPrototype(*this);
}

void Fcn::accept(ASTVisitor &visitor) {
    visitor.visitFcn(*this);
}

llvm::Value* FcnPrototype::accept(ValueVisitor &visitor) {
    return visitor.visitFcnPrototype(*this);
}

llvm::Value* Fcn::accept(ValueVisitor &visitor) {
    return visitor.visitFcn(*this);
}