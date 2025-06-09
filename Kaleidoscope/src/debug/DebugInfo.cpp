
#include "AST/Expr.hpp"
#include "debug/DebugInfo.hpp"

std::unique_ptr<llvm::DIBuilder> DBuilder;
DebugInfo KSDbgInfo;

void DebugInfo::emitLocation(llvm::IRBuilder<>* builder, Expr* expr) {
    if (!expr) {
        return builder->SetCurrentDebugLocation(llvm::DebugLoc());
    }
    llvm::DIScope* scope;
    if (LexicalBlocks.empty()) {
        scope = TheCU;
    } else {
        scope = LexicalBlocks.back();
        builder->SetCurrentDebugLocation(
            llvm::DILocation::get(scope->getContext(), expr->getLine(),
            expr->getCol(), scope));
    }
}

llvm::DIType *DebugInfo::getDoubleTy() {
    if (DblTy)
        return DblTy;

    DblTy = DBuilder->createBasicType("double", 64, llvm::dwarf::DW_ATE_float);
    return DblTy;
}


llvm::DISubroutineType* createFunctionType(unsigned numArgs) {
    llvm::SmallVector<llvm::Metadata*, 8> EltTys;
    llvm::DIType* dblTy = KSDbgInfo.getDoubleTy();

    EltTys.push_back(dblTy);

    for (unsigned i = 0, e = numArgs; i != e; ++i) {
        EltTys.push_back(dblTy);
    }

    return DBuilder->createSubroutineType(DBuilder->getOrCreateTypeArray(EltTys));
}
