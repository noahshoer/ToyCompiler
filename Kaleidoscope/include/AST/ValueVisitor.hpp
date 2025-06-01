#pragma once

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include <llvm/IR/Value.h>
#include <map>

class NumberExpr;
class VariableExpr;
class BinaryExpr;
class CallExpr;

class FcnPrototype;
class Fcn;

class ValueVisitor {
public:
    virtual llvm::Value* visitNumberExpr(NumberExpr &expr) = 0;
    virtual llvm::Value* visitVariableExpr(VariableExpr &expr) = 0;
    virtual llvm::Value* visitBinaryExpr(BinaryExpr &expr) = 0;
    virtual llvm::Value* visitCallExpr(CallExpr &expr) = 0;
    virtual llvm::Value* visitFcnPrototype(FcnPrototype &proto) = 0;
    virtual llvm::Value* visitFcn(Fcn &fcn) = 0;
};

class CodegenVisitor : public ValueVisitor {
public:
    CodegenVisitor(const std::string& moduleName);

    llvm::Value* visitNumberExpr(NumberExpr &expr) override;
    llvm::Value* visitVariableExpr(VariableExpr &expr) override;
    llvm::Value* visitBinaryExpr(BinaryExpr &expr) override;
    llvm::Value* visitCallExpr(CallExpr &expr) override;
    llvm::Value* visitFcnPrototype(FcnPrototype &proto) override;
    llvm::Value* visitFcn(Fcn &fcn) override;


private:
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::LLVMContext> context;

    /// The LLVM module holds functions and global variables, it is
    /// the top-level container for LLVM IR code.
    std::unique_ptr<llvm::Module> module;

    std::map<std::string, llvm::Value *> namedValues;

    void logError(const std::string &message) {
        fprintf(stderr, "Error: %s\n", message.c_str());
    }
};