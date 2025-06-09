#pragma once

#include <memory>
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/IRBuilder.h"

class Expr;

extern std::unique_ptr<llvm::DIBuilder> DBuilder;

struct DebugInfo {
    llvm::DICompileUnit *TheCU;
    llvm::DIType *DblTy;
    std::vector<llvm::DIScope*> LexicalBlocks;

    void emitLocation(llvm::IRBuilder<>* builder, Expr* expr);
    llvm::DIType *getDoubleTy();
};

extern DebugInfo KSDbgInfo;

llvm::DISubroutineType* createFunctionType(unsigned numArgs);
