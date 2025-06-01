#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Node.hpp"

class Expr : public ASTNode {
public:
    virtual ~Expr() = default;

    void accept(ASTVisitor &visitor) override = 0;
    llvm::Value* accept(ValueVisitor &visitor) override = 0;

    const std::string getType() const override = 0;
    virtual std::string toString() const = 0;
};

class NumberExpr : public Expr {
    double value;

public:
    NumberExpr(double val) : value(val) {}

    void accept(ASTVisitor &visitor) override;
    llvm::Value* accept(ValueVisitor &visitor) override;

    const std::string getType() const override {
        return "Number";
    }

    double getValue() const {
        return value;
    }

    std::string toString() const override {
        return std::to_string(value);
    }
};

class VariableExpr : public Expr {
    std::string name;

public:
    VariableExpr(const std::string &varName) : name(varName) {}

    void accept(ASTVisitor &visitor) override;
    llvm::Value* accept(ValueVisitor &visitor) override;

    const std::string getType() const override {
        return "Variable";
    }

    const std::string& getName() const {
        return name;
    }

    std::string toString() const override {
        return name;
    }
};

class BinaryExpr : public Expr {
    char Op;
    std::unique_ptr<Expr> LHS, RHS;

public:
    BinaryExpr(char op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : Op(op), LHS(std::move(lhs)), RHS(std::move(rhs)) {}

    void accept(ASTVisitor &visitor) override;
    llvm::Value* accept(ValueVisitor &visitor) override;

    Expr* getLHS() const {
        return LHS.get();
    }

    Expr* getRHS() const {
        return RHS.get();
    }

    const char getOp() const {
        return Op;
    }

    const std::string getType() const override {
        return "Binary";
    }

    std::string toString() const override {
        return "(" + LHS->toString() + " " + getOp() + " " + RHS->toString() + ")";
    }
};

class CallExpr : public Expr {
    std::string callee;
    std::vector<std::unique_ptr<Expr>> args;

public:
    CallExpr(const std::string &callee, std::vector<std::unique_ptr<Expr>> args)
        : callee(callee), args(std::move(args)) {}

    void accept(ASTVisitor &visitor) override;
    llvm::Value* accept(ValueVisitor &visitor) override;

    std::vector<Expr*> getArgs() const {
        std::vector<Expr*> argsOut;
        for (const auto &arg : args) {
            argsOut.push_back(arg.get());
        }
        return argsOut;
    }

    const std::string& getCalleeName() const {
        return callee;
    }

    int getNumArgs() const {
        return static_cast<int>(args.size());
    }

    const std::string getType() const override {
        return "Call";
    }

    std::string toString() const override {
        std::string result = callee + "(";
        for (const auto &arg : args) {
            result += arg->toString() + ", ";
        }
        if (!args.empty()) {
            result.erase(result.size() - 2); // Remove trailing comma and space
        }
        result += ")";
        return result;
    }
};