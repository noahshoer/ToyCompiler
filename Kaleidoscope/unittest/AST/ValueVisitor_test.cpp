#include "gtest/gtest.h"

#include "llvm/IR/Constants.h"
#include "AST/Expr.hpp"
#include "AST/Fcn.hpp"
#include "AST/ValueVisitor.hpp"
#include "mocks/AST/MockValueVisitor.hpp"

class AcceptOnNodeValueVisitorTest : public testing::Test {
protected:
    void SetUp() override {
        constant = llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), 1);
        value = static_cast<llvm::Value*>(constant);
    }

    llvm::LLVMContext context;
    llvm::ConstantInt* constant;
    llvm::Value* value;
};

TEST_F(AcceptOnNodeValueVisitorTest, VisitNumberExprReturn) {
    MockValueVisitor mockVisitor;
    NumberExpr expr(3.14);

    EXPECT_CALL(mockVisitor, visitNumberExpr(testing::Ref(expr)))
        .WillOnce(testing::Return(value));

    llvm::Value* result = expr.accept(mockVisitor);
    EXPECT_EQ(result, value);
}

TEST_F(AcceptOnNodeValueVisitorTest, VisitVariableExprReturn) {
    MockValueVisitor mockVisitor;
    VariableExpr expr("x");

    EXPECT_CALL(mockVisitor, visitVariableExpr(testing::Ref(expr)))
        .WillOnce(testing::Return(value));

    llvm::Value* result = expr.accept(mockVisitor);
    EXPECT_EQ(result, value);
}

TEST_F(AcceptOnNodeValueVisitorTest, VisitBinaryExprReturn) {
    MockValueVisitor mockVisitor;
    auto lhs = std::make_unique<NumberExpr>(1.0);
    auto rhs = std::make_unique<NumberExpr>(2.0);
    BinaryExpr expr('+', std::move(lhs), std::move(rhs));

    EXPECT_CALL(mockVisitor, visitBinaryExpr(testing::Ref(expr)))
        .WillOnce(testing::Return(value));

    llvm::Value* result = expr.accept(mockVisitor);
    EXPECT_EQ(result, value);
}

TEST_F(AcceptOnNodeValueVisitorTest, VisitCallExprReturn) {
    MockValueVisitor mockVisitor;
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<NumberExpr>(1.0));
    CallExpr expr("foo", std::move(args));

    EXPECT_CALL(mockVisitor, visitCallExpr(testing::Ref(expr)))
        .WillOnce(testing::Return(value));

    llvm::Value* result = expr.accept(mockVisitor);
    EXPECT_EQ(result, value);
}

TEST_F(AcceptOnNodeValueVisitorTest, VisitFcnPrototypeReturn) {
    MockValueVisitor mockVisitor;
    std::vector<std::string> args = {"x", "y"};
    FcnPrototype proto("myFunc", args);

    EXPECT_CALL(mockVisitor, visitFcnPrototype(testing::Ref(proto)))
        .WillOnce(testing::Return(value));

    llvm::Value* result = proto.accept(mockVisitor);
    EXPECT_EQ(result, value);
}

TEST_F(AcceptOnNodeValueVisitorTest, VisitFcnReturn) {
    MockValueVisitor mockVisitor;
    auto proto = std::make_unique<FcnPrototype>("myFunc", std::vector<std::string>{"x", "y"});
    auto body = std::make_unique<NumberExpr>(42.0);
    Fcn fcn(std::move(proto), std::move(body));

    EXPECT_CALL(mockVisitor, visitFcn(testing::Ref(fcn)))
        .WillOnce(testing::Return(value));

    llvm::Value* result = fcn.accept(mockVisitor);
    EXPECT_EQ(result, value);
}