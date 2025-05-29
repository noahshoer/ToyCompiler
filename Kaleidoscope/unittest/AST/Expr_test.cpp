#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "AST/Expr.hpp"

// Mock class for Expr using Google Mock
class MockExpr : public Expr {
public:
    MOCK_METHOD(const std::string, getType, (), (const, override));
    MOCK_METHOD(std::string, toString, (), (const, override));
};

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

// VariableExpr tests
TEST(VariableExprTest, GetTypeReturnsVariable) {
    VariableExpr expr("foo");
    EXPECT_EQ(expr.getType(), "Variable");
}

TEST(VariableExprTest, ToStringReturnsName) {
    VariableExpr expr("bar");
    EXPECT_EQ(expr.toString(), "bar");
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
