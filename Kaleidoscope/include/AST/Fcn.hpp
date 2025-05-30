#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Expr.hpp"
#include "Node.hpp"

class FcnPrototype : public ASTNode {
    std::string name;
    std::vector<std::string> args;

public:
    FcnPrototype(const std::string &Name, std::vector<std::string> Args)
        : name(Name), args(std::move(Args)) {}
        
    void accept(ASTVisitor &visitor) override;

    template<typename R>
    R accept(ASTReturnVisitor<R> &visitor) {
        return visitor.visitFcnPrototype(*this);
    }

    const std::vector<std::string>& getArgs() const {
        return args;
    }

    const std::string &getName() const {
        return name;
    }

    const std::string getType() const override {
        return "FunctionPrototype";
    }
};

class Fcn : public ASTNode {
    std::unique_ptr<FcnPrototype> prototype;
    std::unique_ptr<Expr> body;

public:
    Fcn(std::unique_ptr<FcnPrototype> Prototype, std::unique_ptr<Expr> Body)
        : prototype(std::move(Prototype)), body(std::move(Body)) {}
    
    void accept(ASTVisitor &visitor) override;

    template<typename R>
    R accept(ASTReturnVisitor<R> &visitor) {
        return visitor.visitFcn(*this);
    }

    const std::string getName() const {
        return prototype ? prototype->getName() : "";
    }
    FcnPrototype* getPrototype() const {
        return prototype.get();
    }

    Expr* getBody() const {
        return body.get();
    }

    const std::string getType() const override{
        return "Function";
    }
};