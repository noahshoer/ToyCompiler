#include "gtest/gtest.h"

#include "AST/Expr.hpp"
#include "AST/Fcn.hpp"
#include "AST/ASTVisitor.hpp"
#include "mocks/AST/MockASTVisitor.hpp"

TEST(ASTExprVisitorTest, VisitNumberExpr) {
    MockASTVisitor mockVisitor;
    NumberExpr expr(3.14);

    EXPECT_CALL(mockVisitor, visitNumberExpr(testing::Ref(expr)))
        .Times(1);

    expr.accept(mockVisitor);
}

TEST(ASTExprVisitorTest, VisitVariableExpr) {
    MockASTVisitor mockVisitor;
    VariableExpr expr("x");

    EXPECT_CALL(mockVisitor, visitVariableExpr(testing::Ref(expr)))
        .Times(1);

    expr.accept(mockVisitor);
}

TEST(ASTExprVisitorTest, VisitBinaryExpr) {
    MockASTVisitor mockVisitor;
    auto lhs = std::make_unique<NumberExpr>(1.0);
    auto rhs = std::make_unique<NumberExpr>(2.0);
    BinaryExpr expr('+', std::move(lhs), std::move(rhs));

    EXPECT_CALL(mockVisitor, visitBinaryExpr(testing::Ref(expr)))
        .Times(1);

    expr.accept(mockVisitor);
}

TEST(ASTExprVisitorTest, VisitCallExpr) {
    MockASTVisitor mockVisitor;
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<NumberExpr>(1.0));
    CallExpr expr("foo", std::move(args));

    EXPECT_CALL(mockVisitor, visitCallExpr(testing::Ref(expr)))
        .Times(1);

    expr.accept(mockVisitor);
}

TEST(ASTExprVisitorTest, VisitNumberExprReturn) {
    MockASTReturnVisitor mockReturnVisitor;
    NumberExpr expr(3.14);

    EXPECT_CALL(mockReturnVisitor, visitNumberExpr(testing::Ref(expr)))
        .WillOnce(testing::Return("Visited NumberExpr"));

    std::string result = expr.accept(mockReturnVisitor);
    EXPECT_EQ(result, "Visited NumberExpr");
}

TEST(ASTExprVisitorTest, VisitVariableExprReturn) {
    MockASTReturnVisitor mockReturnVisitor;
    VariableExpr expr("x");

    EXPECT_CALL(mockReturnVisitor, visitVariableExpr(testing::Ref(expr)))
        .WillOnce(testing::Return("Visited VariableExpr"));

    std::string result = expr.accept(mockReturnVisitor);
    EXPECT_EQ(result, "Visited VariableExpr");
}

TEST(ASTExprVisitorTest, VisitBinaryExprReturn) {
    MockASTReturnVisitor mockReturnVisitor;
    auto lhs = std::make_unique<NumberExpr>(1.0);
    auto rhs = std::make_unique<NumberExpr>(2.0);
    BinaryExpr expr('+', std::move(lhs), std::move(rhs));

    EXPECT_CALL(mockReturnVisitor, visitBinaryExpr(testing::Ref(expr)))
        .WillOnce(testing::Return("Visited BinaryExpr"));

    std::string result = expr.accept(mockReturnVisitor);
    EXPECT_EQ(result, "Visited BinaryExpr");
}

TEST(ASTExprVisitorTest, VisitCallExprReturn) {
    MockASTReturnVisitor mockReturnVisitor;
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<NumberExpr>(1.0));
    CallExpr expr("foo", std::move(args));

    EXPECT_CALL(mockReturnVisitor, visitCallExpr(testing::Ref(expr)))
        .WillOnce(testing::Return("Visited CallExpr"));

    std::string result = expr.accept(mockReturnVisitor);
    EXPECT_EQ(result, "Visited CallExpr");
}

TEST(ASTFcnVisitorTest, VisitFcnPrototype) {
    MockASTVisitor mockVisitor;
    std::vector<std::string> args = {"x", "y"};
    FcnPrototype proto("myFunc", args);

    EXPECT_CALL(mockVisitor, visitFcnPrototype(testing::Ref(proto)))
        .Times(1);

    proto.accept(mockVisitor);
}

TEST(ASTFcnVisitorTest, VisitFcn) {
    MockASTVisitor mockVisitor;
    auto proto = std::make_unique<FcnPrototype>("myFunc", std::vector<std::string>{"x", "y"});
    auto body = std::make_unique<NumberExpr>(42.0);
    Fcn fcn(std::move(proto), std::move(body));

    EXPECT_CALL(mockVisitor, visitFcn(testing::Ref(fcn)))
        .Times(1);

    fcn.accept(mockVisitor);
}

TEST(ASTFcnVisitorTest, VisitFcnPrototypeReturn) {
    MockASTReturnVisitor mockReturnVisitor;
    std::vector<std::string> args = {"x", "y"};
    FcnPrototype proto("myFunc", args);

    EXPECT_CALL(mockReturnVisitor, visitFcnPrototype(testing::Ref(proto)))
        .WillOnce(testing::Return("Visited FcnPrototype"));

    std::string result = proto.accept(mockReturnVisitor);
    EXPECT_EQ(result, "Visited FcnPrototype");
}

TEST(ASTFcnVisitorTest, VisitFcnReturn) {
    MockASTReturnVisitor mockReturnVisitor;
    auto proto = std::make_unique<FcnPrototype>("myFunc", std::vector<std::string>{"x", "y"});
    auto body = std::make_unique<NumberExpr>(42.0);
    Fcn fcn(std::move(proto), std::move(body));

    EXPECT_CALL(mockReturnVisitor, visitFcn(testing::Ref(fcn)))
        .WillOnce(testing::Return("Visited Fcn"));

    std::string result = fcn.accept(mockReturnVisitor);
    EXPECT_EQ(result, "Visited Fcn");
}