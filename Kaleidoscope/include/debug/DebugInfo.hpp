#pragma once

#include <memory>
#include "llvm/IR/DIBuilder.h"

static std::unique_ptr<llvm::DIBuilder> DBuilder;

struct DebugInfo {
  llvm::DICompileUnit *TheCU;
  llvm::DIType *DblTy;

  llvm::DIType *getDoubleTy();
};

static struct DebugInfo KSDbgInfo;
