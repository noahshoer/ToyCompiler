#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "AST/Expr.hpp"
#include "mocks/AST/MockExpr.hpp"

// MockExpr tests
TEST(MockExprTest, InterfaceTest) {
    MockExpr mock;

    EXPECT_CALL(mock, getType()).WillOnce(testing::Return("MockType"));
    EXPECT_CALL(mock, toString()).WillOnce(testing::Return("MockString"));

    EXPECT_EQ(mock.getType(), "MockType");
    EXPECT_EQ(mock.toString(), "MockString");
}

// NumberExpr tests
TEST(NumberExprTest, GetTypeReturnsNumber) {
    NumberExpr expr(42.0);
    EXPECT_EQ(expr.getType(), "Number");
}

TEST(NumberExprTest, ToStringReturnsValue) {
    NumberExpr expr(3.14);
    EXPECT_EQ(expr.toString(), std::to_string(3.14));
}

TEST(NumberExprTest, GetValueReturnsCorrectValue) {
    NumberExpr expr(1.618);
    EXPECT_EQ(expr.getValue(), 1.618);
}

// VariableExpr tests
TEST(VariableExprTest, GetTypeReturnsVariable) {
    VariableExpr expr("foo");
    EXPECT_EQ(expr.getType(), "Variable");
}

TEST(VariableExprTest, ToStringReturnsName) {
    VariableExpr expr("bar");
    EXPECT_EQ(expr.toString(), "bar");
}

TEST(VariableExprTest, GetNameReturnsCorrectName) {
    VariableExpr expr("baz");
    EXPECT_EQ(expr.getName(), "baz");
}

// BinaryExpr tests
TEST(BinaryExprTest, GetTypeReturnsBinary) {
    auto lhs = std::make_unique<NumberExpr>(1.0);
    auto rhs = std::make_unique<NumberExpr>(2.0);
    BinaryExpr expr('+', std::move(lhs), std::move(rhs));
    EXPECT_EQ(expr.getType(), "Binary");
}

TEST(BinaryExprTest, ToStringReturnsCorrectFormat) {
    auto lhs = std::make_unique<NumberExpr>(1.0);
    auto rhs = std::make_unique<NumberExpr>(2.0);
    BinaryExpr expr('*', std::move(lhs), std::move(rhs));
    EXPECT_EQ(expr.toString(), "(1.000000 * 2.000000)");
}

TEST(BinaryExprTest, ToStringWithVariable) {
    auto lhs = std::make_unique<VariableExpr>("x");
    auto rhs = std::make_unique<NumberExpr>(5.0);
    BinaryExpr expr('-', std::move(lhs), std::move(rhs));
    EXPECT_EQ(expr.toString(), "(x - 5.000000)");
}

TEST(BinaryExprTest, GetLHSReturnsCorrectExpr) {
    auto lhs = std::make_unique<NumberExpr>(3.0);
    auto rhs = std::make_unique<NumberExpr>(4.0);
    BinaryExpr expr('+', std::move(lhs), std::move(rhs));
    EXPECT_EQ(expr.getLHS()->toString(), "3.000000");
}

TEST(BinaryExprTest, GetRHSReturnsCorrectExpr) {
    auto lhs = std::make_unique<NumberExpr>(6.0);
    auto rhs = std::make_unique<VariableExpr>("y");
    BinaryExpr expr('/', std::move(lhs), std::move(rhs));
    EXPECT_EQ(expr.getRHS()->toString(), "y");
}

TEST(BinaryExprTest, GetOpReturnsCorrectOperator) {
    auto lhs = std::make_unique<NumberExpr>(8.0);
    auto rhs = std::make_unique<NumberExpr>(2.0);
    BinaryExpr expr('-', std::move(lhs), std::move(rhs));
    EXPECT_EQ(expr.getOp(), '-');
}

// CallExpr tests
TEST(CallExprTest, GetTypeReturnsCall) {
    std::vector<std::unique_ptr<Expr>> args;
    CallExpr expr("foo", std::move(args));
    EXPECT_EQ(expr.getType(), "Call");
}

TEST(CallExprTest, ToStringNoArgs) {
    std::vector<std::unique_ptr<Expr>> args;
    CallExpr expr("bar", std::move(args));
    EXPECT_EQ(expr.toString(), "bar()");
}

TEST(CallExprTest, ToStringWithArgs) {
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<NumberExpr>(1.0));
    args.push_back(std::make_unique<VariableExpr>("y"));
    CallExpr expr("baz", std::move(args));
    EXPECT_EQ(expr.toString(), "baz(1.000000, y)");
}

TEST(CallExprTest, ToStringWithNestedExprs) {
    std::vector<std::unique_ptr<Expr>> args;
    auto lhs = std::make_unique<NumberExpr>(2.0);
    auto rhs = std::make_unique<NumberExpr>(3.0);
    args.push_back(std::make_unique<BinaryExpr>('+', std::move(lhs), std::move(rhs)));
    CallExpr expr("sum", std::move(args));
    EXPECT_EQ(expr.toString(), "sum((2.000000 + 3.000000))");
}

TEST(CallExprTest, GetCalleeNameReturnsCorrectName) {
    std::vector<std::unique_ptr<Expr>> args;
    CallExpr expr("calculate", std::move(args));
    EXPECT_EQ(expr.getCalleeName(), "calculate");
}

TEST(CallExprTest, GetArgsReturnsCorrectArgs) {
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<NumberExpr>(10.0));
    args.push_back(std::make_unique<VariableExpr>("x"));
    CallExpr expr("process", std::move(args));

    auto argList = expr.getArgs();
    ASSERT_EQ(argList.size(), 2);
    EXPECT_EQ(argList[0]->toString(), "10.000000");
    EXPECT_EQ(argList[1]->toString(), "x");
}

TEST(CallExprTest, GetNumArgsReturnsCorrectCount) {
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<NumberExpr>(1.0));
    args.push_back(std::make_unique<VariableExpr>("y"));
    CallExpr expr("func", std::move(args));
    EXPECT_EQ(expr.getNumArgs(), 2);
}