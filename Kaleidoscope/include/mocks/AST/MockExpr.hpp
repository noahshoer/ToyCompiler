#pragma once

#include "AST/Expr.hpp"
#include "gmock/gmock.h"

class MockExpr : public Expr {
public:
    MOCK_METHOD(void, accept, (ASTVisitor &visitor), (override));
    MOCK_METHOD(llvm::Value*, accept, (ValueVisitor &visitor), (override));
    MOCK_METHOD(const std::string, getType, (), (const, override));
    MOCK_METHOD(std::string, toString, (), (const, override));
};