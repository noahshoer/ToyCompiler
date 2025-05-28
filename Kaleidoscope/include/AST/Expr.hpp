#pragma once
#include <memory>
#include <string>
#include <vector>

class Expr {
public:
    virtual ~Expr() = default;

    // Accept a visitor for this expression
    virtual void accept(class ExprVisitor &visitor) = 0;

    // Get the type of the expression
    virtual const char* getType() const = 0;

    // Get the string representation of the expression
    virtual std::string toString() const = 0;
};

class NumberExpr : public Expr {
    double value;

public:
    NumberExpr(double val) : value(val) {}

    void accept(ExprVisitor &visitor) override;

    const char* getType() const override {
        return "Number";
    }

    std::string toString() const override {
        return std::to_string(value);
    }
};

class VariableExpr : public Expr {
    std::string name;

public:
    VariableExpr(const std::string &varName) : name(varName) {}

    void accept(ExprVisitor &visitor) override;

    const char* getType() const override {
        return "Variable";
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

    void accept(ExprVisitor &visitor) override;

    const char* getType() const override {
        return "Binary";
    }

    std::string toString() const override {
        return "(" + LHS->toString() + " " + Op + " " + RHS->toString() + ")";
    }
};

class CallExpr : public Expr {
    std::string callee;
    std::vector<std::unique_ptr<Expr>> args;

public:
    CallExpr(const std::string &callee, std::vector<std::unique_ptr<Expr>> args)
        : callee(callee), args(std::move(args)) {}

    void accept(ExprVisitor &visitor) override;

    const char* getType() const override {
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