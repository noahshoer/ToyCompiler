#pragma once

#include <cassert>
#include <string>

#include "llvm/IR/Value.h"

#include "debug/SourceLocation.hpp"

class ASTVisitor;
class ValueVisitor;

class ASTNode {
    SourceLocation Loc;

public:
    virtual ~ASTNode() = default;

    virtual void accept(ASTVisitor &visitor) = 0;
    virtual llvm::Value* accept(ValueVisitor &visitor) = 0;

    void setSourceLoc(SourceLocation loc) {
        Loc = loc;
    }

    int getLine() const { return Loc.Line; }
    int getCol() const { return Loc.Col; }

    virtual const std::string getType() const = 0;
};