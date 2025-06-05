#pragma once

#include "Expr.hpp"
#include "Fcn.hpp"

class ASTVisitor {
public:
    virtual void visitNumberExpr(NumberExpr &expr) = 0;
    virtual void visitVariableExpr(VariableExpr &expr) = 0;
    virtual void visitBinaryExpr(BinaryExpr &expr) = 0;
    virtual void visitUnaryExpr(UnaryExpr &expr) = 0;
    virtual void visitCallExpr(CallExpr &expr) = 0;
    virtual void visitIfExpr(IfExpr &expr) = 0;
    virtual void visitForExpr(ForExpr &expr) = 0;
    virtual void visitVarExpr(VarExpr &expr) = 0;

    virtual void visitFcnPrototype(FcnPrototype &proto) = 0;
    virtual void visitFcn(Fcn &fcn) = 0;
};