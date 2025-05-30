#pragma once

#include "gmock/gmock.h"
#include "AST/ASTVisitor.hpp"


class MockASTVisitor : public ASTVisitor {
public:
    MOCK_METHOD(void, visitNumberExpr, (NumberExpr &expr), (override));
    MOCK_METHOD(void, visitVariableExpr, (VariableExpr &expr), (override));
    MOCK_METHOD(void, visitBinaryExpr, (BinaryExpr &expr), (override));
    MOCK_METHOD(void, visitCallExpr, (CallExpr &expr), (override));
    MOCK_METHOD(void, visitFcnPrototype, (FcnPrototype &proto), (override));
    MOCK_METHOD(void, visitFcn, (Fcn &fcn), (override));
};

class MockASTReturnVisitor : public ASTReturnVisitor<std::string> {
public:
    MOCK_METHOD(std::string, visitNumberExpr, (NumberExpr &expr), (override));
    MOCK_METHOD(std::string, visitVariableExpr, (VariableExpr &expr), (override));
    MOCK_METHOD(std::string, visitBinaryExpr, (BinaryExpr &expr), (override));
    MOCK_METHOD(std::string, visitCallExpr, (CallExpr &expr), (override));
    MOCK_METHOD(std::string, visitFcnPrototype, (FcnPrototype &proto), (override));
    MOCK_METHOD(std::string, visitFcn, (Fcn &fcn), (override));
};