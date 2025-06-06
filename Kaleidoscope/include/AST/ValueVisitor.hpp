#pragma once

#include <map>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include <llvm/IR/Value.h>

#include "Expr.hpp"
#include "Fcn.hpp"

class ValueVisitor {
public:
    virtual llvm::Value* visitNumberExpr(NumberExpr &expr) = 0;
    virtual llvm::Value* visitVariableExpr(VariableExpr &expr) = 0;
    virtual llvm::Value* visitBinaryExpr(BinaryExpr &expr) = 0;
    virtual llvm::Value* visitUnaryExpr(UnaryExpr &expr) = 0;
    virtual llvm::Value* visitCallExpr(CallExpr &expr) = 0;
    virtual llvm::Value* visitIfExpr(IfExpr &expr) = 0;
    virtual llvm::Value* visitForExpr(ForExpr &expr) = 0;
    virtual llvm::Value* visitVarExpr(VarExpr &expr) = 0;

    virtual llvm::Value* visitFcnPrototype(FcnPrototype &proto) = 0;
    virtual llvm::Value* visitFcn(Fcn &fcn) = 0;
};

class CodegenVisitor : public ValueVisitor {
public:
    CodegenVisitor(llvm::LLVMContext* ctx, llvm::Module* mod, llvm::IRBuilder<>* build)
        : context(ctx), module(mod), builder(build) {}

    llvm::Value* visitNumberExpr(NumberExpr &expr) override;
    llvm::Value* visitVariableExpr(VariableExpr &expr) override;
    llvm::Value* visitBinaryExpr(BinaryExpr &expr) override;
    llvm::Value* visitUnaryExpr(UnaryExpr &expr) override;
    llvm::Value* visitCallExpr(CallExpr &expr) override;
    llvm::Value* visitIfExpr(IfExpr &expr) override;
    llvm::Value* visitForExpr(ForExpr &expr) override;
    llvm::Value* visitVarExpr(VarExpr &expr) override;

    llvm::Value* visitFcnPrototype(FcnPrototype &proto) override;
    llvm::Value* visitFcn(Fcn &fcn) override;

    void setFPM(llvm::FunctionPassManager* FPM) {
        fpm = FPM;
    }

    void setFAM(llvm::FunctionAnalysisManager* FAM) {
        fam = FAM;
    }

    void setNamedValue(const std::string& name, llvm::AllocaInst* allocaInst) {
        namedValues[name] = allocaInst;
    }

    llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* function, 
                                                llvm::StringRef varName) {
        llvm::IRBuilder<> tmpB(&function->getEntryBlock(),
                                function->getEntryBlock().begin());
        return tmpB.CreateAlloca(llvm::Type::getDoubleTy(*context),
                                    nullptr, varName);
    }

private:
    llvm::IRBuilder<>* builder;
    llvm::LLVMContext* context;
    llvm::FunctionPassManager* fpm;
    llvm::FunctionAnalysisManager* fam;

    /// The LLVM module holds functions and global variables, it is
    /// the top-level container for LLVM IR code.
    llvm::Module* module;

    // Currently only mantains function arguments and loop
    // induction variables
    std::map<std::string, llvm::AllocaInst*> namedValues;

    llvm::Value* logError(const std::string &message) {
        (void)message;
        // fprintf(stderr, "Error: %s\n", message.c_str());
        return nullptr;
    }
};