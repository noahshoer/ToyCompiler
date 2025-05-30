#pragma once

#include <cassert>
#include <string>
class ASTVisitor;

template<typename R>
class ASTReturnVisitor;

class ASTNode {
public:
    virtual ~ASTNode() = default;

    virtual void accept(ASTVisitor &visitor) = 0;

    template <typename R>
    R accept(ASTReturnVisitor<R>& visitor) {
        assert(false && "accept not implemented for this node type");
    }

    virtual const std::string getType() const = 0;
};