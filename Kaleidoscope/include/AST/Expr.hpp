#pragma once

#include <format>
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

using ExprUPtr = std::unique_ptr<Expr>;

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
        return std::format("{:.15g}", value);
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
    ExprUPtr LHS, RHS;

public:
    BinaryExpr(char op, ExprUPtr lhs, ExprUPtr rhs)
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

class UnaryExpr : public Expr {
    char op;
    ExprUPtr operand;

public:
    UnaryExpr(char Op, ExprUPtr Operand)
        : op(Op), operand(std::move(Operand)) {}

    void accept(ASTVisitor &visitor) override;
    llvm::Value* accept(ValueVisitor &visitor) override;

    const char getOp() const {
        return op;
    }

    Expr* getOperand() const {
        return operand.get();
    }

    const std::string getType() const override {
        return "Unary";
    }

    std::string toString() const override {
        return getOp() + operand->toString();
    }

};

class CallExpr : public Expr {
    std::string callee;
    std::vector<ExprUPtr> args;

public:
    CallExpr(const std::string &callee, std::vector<ExprUPtr> args)
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

class IfExpr : public Expr {
    ExprUPtr Cond, Then, Else;

public:
    IfExpr(ExprUPtr aCond, ExprUPtr aThen,
            ExprUPtr aElse) 
        : Cond(std::move(aCond)), Then(std::move(aThen)), Else(std::move(aElse)) {}

    void accept(ASTVisitor &visitor) override;
    llvm::Value* accept(ValueVisitor &visitor) override;

    Expr* getCond() const {
        return Cond.get();
    }

    Expr* getThen() const {
        return Then.get();
    }

    Expr* getElse() const {
        return Else.get();
    }

    const std::string getType() const override {
        return "If-Then-Else";
    }

    std::string toString() const override {
        std::string result = "if " + Cond->toString() + " then\n";
        result += "\t" + Then->toString() + "\n";
        result += "else\n";
        result += "\t" + Else->toString();
        return result;
    }
};

class ForExpr : public Expr {
    std::string varName;
    ExprUPtr start, end, step, body;

public:
    ForExpr(const std::string& aVarName, ExprUPtr aStart,
            ExprUPtr aEnd, ExprUPtr aStep,
            ExprUPtr aBody)
        : varName(aVarName), start(std::move(aStart)), end(std::move(aEnd)),
            step(std::move(aStep)), body(std::move(aBody)) {}

    void accept(ASTVisitor &visitor) override;
    llvm::Value* accept(ValueVisitor &visitor) override;

    const std::string& getVarName() const {
        return varName;
    }

    Expr* getStart() const {
        return start.get();
    }

    Expr* getEnd() const {
        return end.get();
    }

    Expr* getStep() const {
        return step.get();
    }

    Expr* getBody() const {
        return body.get();
    }

    const std::string getType() const override {
        return "ForLoop";
    }

    std::string toString() const override {
        std::string result = "for " + start->toString() + ", "
            + end->toString() + ", " + step->toString() + "\n"
            + "\t" + body->toString();
        return result;
    }
};

using VarNameVector = std::vector<std::pair<std::string, ExprUPtr>>;
class VarExpr : public Expr {
    VarNameVector varNames;
    ExprUPtr body;

public:
    VarExpr(VarNameVector VarNames, ExprUPtr Body)
        : varNames(std::move(VarNames)), body(std::move(Body)) {}

    void accept(ASTVisitor &visitor) override;
    llvm::Value* accept(ValueVisitor &visitor) override;

    const std::string getType() const override {
        return "Var";
    }

    std::string toString() const override {
        std::string result = "var ";
        for (const auto& var : varNames) {
            result += var.first;
            if (var.second) {
                result += " = " + var.second->toString();
            }
            result += ", ";
        }

        result = result.substr(0, result.size() - 2);
        if (body) {
            result += " in\n" + body->toString();
        }
        return result;
    }

    std::vector<std::pair<std::string, Expr*>> getVarNames() const {
        std::vector<std::pair<std::string, Expr*>> result;
        for (const auto& var : varNames) {
            result.push_back(std::make_pair(var.first, var.second.get()));
        }
        return result;
    }

    Expr* getBody() const {
        return body.get();
    }
};