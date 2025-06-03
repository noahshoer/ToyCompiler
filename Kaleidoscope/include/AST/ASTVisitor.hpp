#pragma once

class NumberExpr;
class VariableExpr;
class BinaryExpr;
class CallExpr;
class IfExpr;
class ForExpr;

class FcnPrototype;
class Fcn;

class ASTVisitor {
public:
    virtual void visitNumberExpr(NumberExpr &expr) = 0;
    virtual void visitVariableExpr(VariableExpr &expr) = 0;
    virtual void visitBinaryExpr(BinaryExpr &expr) = 0;
    virtual void visitCallExpr(CallExpr &expr) = 0;
    virtual void visitIfExpr(IfExpr &expr) = 0;
    virtual void visitForExpr(ForExpr &expr) = 0;

    virtual void visitFcnPrototype(FcnPrototype &proto) = 0;
    virtual void visitFcn(Fcn &fcn) = 0;
};