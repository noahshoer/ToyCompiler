#pragma once

#include <cassert>
#include <string>

#include "llvm/IR/Value.h"

class ASTVisitor;
class ValueVisitor;

class ASTNode {
public:
    virtual ~ASTNode() = default;

    virtual void accept(ASTVisitor &visitor) = 0;
    virtual llvm::Value* accept(ValueVisitor &visitor) = 0;

    virtual const std::string getType() const = 0;
};