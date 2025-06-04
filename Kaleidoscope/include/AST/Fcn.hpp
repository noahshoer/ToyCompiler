#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Expr.hpp"
#include "Node.hpp"

class FcnPrototype : public ASTNode {
    std::string name;
    std::vector<std::string> args;
    bool isOperator;
    unsigned binaryPrecedence;

public:
    FcnPrototype(const std::string &Name, std::vector<std::string> Args,
                    bool IsOperator = false, unsigned Prec = 0)
        : name(Name), args(std::move(Args)), 
            isOperator(IsOperator), binaryPrecedence(Prec) {}
        
    void accept(ASTVisitor &visitor) override;
    llvm::Value* accept(ValueVisitor &visitor) override;

    const std::vector<std::string>& getArgs() const {
        return args;
    }

    const std::string &getName() const {
        return name;
    }

    const std::string getType() const override {
        return "FunctionPrototype";
    }

    bool isUnaryOp() const { return isOperator && args.size() == 1; }
    bool isBinaryOp() const { return isOperator && args.size() == 2; }

    char getOperatorName() const {
        assert(isUnaryOp() || isBinaryOp() && 
            "Not a binary or unary operator");
        return name[name.size() - 1];
    }

    unsigned getBinaryPrecedence() const { return binaryPrecedence; }
};

class Fcn : public ASTNode {
    std::unique_ptr<FcnPrototype> prototype;
    std::unique_ptr<Expr> body;

public:
    Fcn(std::unique_ptr<FcnPrototype> Prototype, std::unique_ptr<Expr> Body)
        : prototype(std::move(Prototype)), body(std::move(Body)) {}
    
    void accept(ASTVisitor &visitor) override;
    llvm::Value* accept(ValueVisitor &visitor) override;

    const std::string getName() const {
        return prototype ? prototype->getName() : "";
    }

    FcnPrototype* getPrototype() const {
        return prototype.get();
    }

    std::unique_ptr<FcnPrototype> releasePrototype() {
        return std::move(prototype);
    }

    Expr* getBody() const {
        return body.get();
    }

    const std::string getType() const override{
        return "Function";
    }
};