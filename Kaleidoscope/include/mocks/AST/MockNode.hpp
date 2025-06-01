#pragma once

#include "gmock/gmock.h"
#include "AST/Node.hpp"

class MockASTNode : public ASTNode {
public:
    MOCK_METHOD(void, accept, (ASTVisitor &visitor), (override));
    MOCK_METHOD(llvm::Value*, accept, (ValueVisitor &visitor), (override));
    MOCK_METHOD(const std::string, getType, (), (const, override));
};