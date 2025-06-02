#pragma once

#include "gmock/gmock.h"
#include "llvm/IR/LLVMContext.h"
#include "AST/ValueVisitor.hpp"

class MockValueVisitor : public ValueVisitor {
public:
    MOCK_METHOD(llvm::Value*, visitNumberExpr, (NumberExpr &expr), (override));
    MOCK_METHOD(llvm::Value*, visitVariableExpr, (VariableExpr &expr), (override));
    MOCK_METHOD(llvm::Value*, visitBinaryExpr, (BinaryExpr &expr), (override));
    MOCK_METHOD(llvm::Value*, visitCallExpr, (CallExpr &expr), (override));
    MOCK_METHOD(llvm::Value*, visitIfExpr, (IfExpr &expr), (override));

    MOCK_METHOD(llvm::Value*, visitFcnPrototype, (FcnPrototype &proto), (override));
    MOCK_METHOD(llvm::Value*, visitFcn, (Fcn &fcn), (override));
};

class MockedValueVisitorTest : public testing::Test {
protected:
    void SetUp() override {
        constant = llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), 1);
        value = static_cast<llvm::Value*>(constant);
    }

    llvm::LLVMContext context;
    llvm::ConstantInt* constant;
    llvm::Value* value;
};