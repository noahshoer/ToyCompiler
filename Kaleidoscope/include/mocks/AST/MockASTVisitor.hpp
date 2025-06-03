#pragma once

#include "gmock/gmock.h"
#include "AST/ASTVisitor.hpp"


class MockASTVisitor : public ASTVisitor {
public:
    MOCK_METHOD(void, visitNumberExpr, (NumberExpr &expr), (override));
    MOCK_METHOD(void, visitVariableExpr, (VariableExpr &expr), (override));
    MOCK_METHOD(void, visitBinaryExpr, (BinaryExpr &expr), (override));
    MOCK_METHOD(void, visitCallExpr, (CallExpr &expr), (override));
    MOCK_METHOD(void, visitIfExpr, (IfExpr &expr), (override));
    MOCK_METHOD(void, visitForExpr, (ForExpr &expr), (override));

    MOCK_METHOD(void, visitFcnPrototype, (FcnPrototype &proto), (override));
    MOCK_METHOD(void, visitFcn, (Fcn &fcn), (override));
};
