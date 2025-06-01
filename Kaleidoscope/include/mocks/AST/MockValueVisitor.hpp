#pragma once

#include "gmock/gmock.h"
#include "AST/ValueVisitor.hpp"

class MockValueVisitor : public ValueVisitor {
public:
    MOCK_METHOD(llvm::Value*, visitNumberExpr, (NumberExpr &expr), (override));
    MOCK_METHOD(llvm::Value*, visitVariableExpr, (VariableExpr &expr), (override));
    MOCK_METHOD(llvm::Value*, visitBinaryExpr, (BinaryExpr &expr), (override));
    MOCK_METHOD(llvm::Value*, visitCallExpr, (CallExpr &expr), (override));
    MOCK_METHOD(llvm::Value*, visitFcnPrototype, (FcnPrototype &proto), (override));
    MOCK_METHOD(llvm::Value*, visitFcn, (Fcn &fcn), (override));
};