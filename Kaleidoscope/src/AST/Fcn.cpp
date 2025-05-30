#include "AST/ASTVisitor.hpp"
#include "AST/Fcn.hpp"

void FcnPrototype::accept(ASTVisitor &visitor) {
    visitor.visitFcnPrototype(*this);
}

void Fcn::accept(ASTVisitor &visitor) {
    visitor.visitFcn(*this);
}