#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "AST/Expr.hpp"
#include "mocks/AST/MockExpr.hpp"
#include "mocks/AST/MockASTVisitor.hpp"
#include "mocks/AST/MockValueVisitor.hpp"

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

TEST(NumberExprTest, AcceptASTVisitor) {
    MockASTVisitor mockVisitor;
    NumberExpr expr(3.14);

    EXPECT_CALL(mockVisitor, visitNumberExpr(testing::Ref(expr)))
        .Times(1);

    expr.accept(mockVisitor);
}

TEST_F(MockedValueVisitorTest, VisitNumberExpr) {
    MockValueVisitor mockVisitor;
    NumberExpr expr(3.14);

    EXPECT_CALL(mockVisitor, visitNumberExpr(testing::Ref(expr)))
        .WillOnce(testing::Return(value));

    llvm::Value* result = expr.accept(mockVisitor);
    EXPECT_EQ(result, value);
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

TEST(VariableExprTest, AcceptASTVisitor) {
    MockASTVisitor mockVisitor;
    VariableExpr expr("x");

    EXPECT_CALL(mockVisitor, visitVariableExpr(testing::Ref(expr)))
        .Times(1);

    expr.accept(mockVisitor);
}

TEST_F(MockedValueVisitorTest, VisitVariableExpr) {
    MockValueVisitor mockVisitor;
    VariableExpr expr("x");

    EXPECT_CALL(mockVisitor, visitVariableExpr(testing::Ref(expr)))
        .WillOnce(testing::Return(value));

    llvm::Value* result = expr.accept(mockVisitor);
    EXPECT_EQ(result, value);
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

TEST(BinaryExprTest, AcceptASTVisitor) {
    MockASTVisitor mockVisitor;
    auto lhs = std::make_unique<NumberExpr>(1.0);
    auto rhs = std::make_unique<NumberExpr>(2.0);
    BinaryExpr expr('+', std::move(lhs), std::move(rhs));

    EXPECT_CALL(mockVisitor, visitBinaryExpr(testing::Ref(expr)))
        .Times(1);

    expr.accept(mockVisitor);
}

TEST_F(MockedValueVisitorTest, VisitBinaryExpr) {
    MockValueVisitor mockVisitor;
    auto lhs = std::make_unique<NumberExpr>(1.0);
    auto rhs = std::make_unique<NumberExpr>(2.0);
    BinaryExpr expr('+', std::move(lhs), std::move(rhs));

    EXPECT_CALL(mockVisitor, visitBinaryExpr(testing::Ref(expr)))
        .WillOnce(testing::Return(value));

    llvm::Value* result = expr.accept(mockVisitor);
    EXPECT_EQ(result, value);
}

// UnaryExppr tests
TEST(UnaryExprTest, GetTypeReturnsBinary) {
    auto operand = std::make_unique<NumberExpr>(1.0);
    UnaryExpr expr('!', std::move(operand));
    EXPECT_EQ(expr.getType(), "Unary");
}

TEST(UnaryExprTest, ToStringReturnsCorrectFormat) {
    auto operand = std::make_unique<NumberExpr>(1.0);
    UnaryExpr expr('!', std::move(operand));
    EXPECT_EQ(expr.toString(), "!1.000000");
}

TEST(UnaryExprTest, ToStringWithVariable) {
    auto operand = std::make_unique<VariableExpr>("x");
    UnaryExpr expr('!', std::move(operand));
    EXPECT_EQ(expr.toString(), "!x");
}

TEST(UnaryExprTest, GetOpReturnsCorrectOperator) {
    auto operand = std::make_unique<VariableExpr>("x");
    auto opPtr = operand.get();
    UnaryExpr expr('!', std::move(operand));
    EXPECT_EQ(expr.getOp(), '!');
}

TEST(UnaryExprTest, GetOperand) {
    auto operand = std::make_unique<VariableExpr>("x");
    auto opPtr = operand.get();
    UnaryExpr expr('!', std::move(operand));
    EXPECT_EQ(expr.getOperand(), opPtr);
}

TEST(UnaryExprTest, AcceptASTVisitor) {
    MockASTVisitor mockVisitor;
    auto operand = std::make_unique<NumberExpr>(1.0);
    UnaryExpr expr('!', std::move(operand));

    EXPECT_CALL(mockVisitor, visitUnaryExpr(testing::Ref(expr)))
        .Times(1);

    expr.accept(mockVisitor);
}

TEST_F(MockedValueVisitorTest, VisitUnaryExpr) {
    MockValueVisitor mockVisitor;
    auto operand = std::make_unique<NumberExpr>(1.0);
    UnaryExpr expr('!', std::move(operand));

    EXPECT_CALL(mockVisitor, visitUnaryExpr(testing::Ref(expr)))
        .WillOnce(testing::Return(value));

    llvm::Value* result = expr.accept(mockVisitor);
    EXPECT_EQ(result, value);
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

TEST(CallExprTest, AcceptASTVisitor) {
    MockASTVisitor mockVisitor;
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<NumberExpr>(1.0));
    CallExpr expr("foo", std::move(args));

    EXPECT_CALL(mockVisitor, visitCallExpr(testing::Ref(expr)))
        .Times(1);

    expr.accept(mockVisitor);
}

TEST_F(MockedValueVisitorTest, VisitCallExpr) {
    MockValueVisitor mockVisitor;
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<NumberExpr>(1.0));
    CallExpr expr("foo", std::move(args));

    EXPECT_CALL(mockVisitor, visitCallExpr(testing::Ref(expr)))
        .WillOnce(testing::Return(value));

    llvm::Value* result = expr.accept(mockVisitor);
    EXPECT_EQ(result, value);
}

class IfExprTest : public MockedValueVisitorTest {
protected:
    void SetUp() override {
        MockedValueVisitorTest::SetUp();
        auto lhs = std::make_unique<NumberExpr>(6.0);
        auto rhs = std::make_unique<VariableExpr>("y");
        auto cond = std::make_unique<BinaryExpr>('+', std::move(lhs), std::move(rhs));
        exprCond = cond.get();
        auto then = std::make_unique<NumberExpr>(1);
        exprThen = then.get();
        auto aElse = std::make_unique<NumberExpr>(2);
        exprElse = aElse.get();
        expr = std::make_unique<IfExpr>(std::move(cond), std::move(then), std::move(aElse));
    }

    std::unique_ptr<IfExpr> expr;
    BinaryExpr* exprCond;
    NumberExpr* exprThen;
    NumberExpr* exprElse;
};

TEST_F(IfExprTest, GetTypeReturnsIfThenElse) {
    EXPECT_EQ(expr->getType(), "If-Then-Else");
}

TEST_F(IfExprTest, ToString) {
    EXPECT_EQ(expr->toString(), "if " + exprCond->toString() + " then\n"
        + "\t" + exprThen->toString() + "\n"
        + "else\n"
        + "\t" + exprElse->toString());
}

TEST_F(IfExprTest, GetCond) {
    EXPECT_EQ(expr->getCond(), exprCond);
}

TEST_F(IfExprTest, GetThen) {
    EXPECT_EQ(expr->getThen(), exprThen);
}

TEST_F(IfExprTest, GetElse) {
    EXPECT_EQ(expr->getElse(), exprElse);
}

TEST_F(IfExprTest, AcceptASTVisitor) {
    MockASTVisitor mockVisitor;

    EXPECT_CALL(mockVisitor, visitIfExpr(testing::Ref(*expr)))
        .Times(1);

    expr->accept(mockVisitor); 
}

TEST_F(IfExprTest, VisitIfExpr) {
    MockValueVisitor mockVisitor;

    EXPECT_CALL(mockVisitor, visitIfExpr(testing::Ref(*expr)))
        .WillOnce(testing::Return(value));

    llvm::Value* result = expr->accept(mockVisitor);
    EXPECT_EQ(result, value);
}

class ForExprTest : public MockedValueVisitorTest {
protected:
    void SetUp() override {
        MockedValueVisitorTest::SetUp();
        auto Start = std::make_unique<NumberExpr>(0);
        start = Start.get();
        auto End = std::make_unique<BinaryExpr>('<', 
            std::make_unique<VariableExpr>(loopId), 
            std::make_unique<NumberExpr>(10));
        end = End.get();
        auto Step = std::make_unique<NumberExpr>(1);
        step = Step.get();
        std::vector<std::unique_ptr<Expr>> args;
        args.push_back(std::make_unique<NumberExpr>(1.0));
        auto Body = std::make_unique<CallExpr>("foo", std::move(args));
        body = Body.get();
        expr = std::make_unique<ForExpr>(loopId, std::move(Start), std::move(End),
                std::move(Step), std::move(Body));
    }

    std::string loopId = "i";
    std::unique_ptr<ForExpr> expr;
    Expr* start;
    Expr* end;
    Expr* step;
    Expr* body;
};

TEST_F(ForExprTest, GetTypeReturnsForLoop) {
    EXPECT_EQ(expr->getType(), "ForLoop");
}

TEST_F(ForExprTest, ToString) {
    EXPECT_EQ(expr->toString(), "for " + start->toString() + ", "
        + end->toString() + ", " + step->toString() + "\n"
        + "\t" + body->toString());
}

TEST_F(ForExprTest, GetVarName) {
    EXPECT_EQ(expr->getVarName(), loopId);
}

TEST_F(ForExprTest, GetStart) {
    EXPECT_EQ(expr->getStart(), start);
}

TEST_F(ForExprTest, GetEnd) {
    EXPECT_EQ(expr->getEnd(), end);
}

TEST_F(ForExprTest, GetStep) {
    EXPECT_EQ(expr->getStep(), step);
}

TEST_F(ForExprTest, GetBody) {
    EXPECT_EQ(expr->getBody(), body);
}

TEST_F(ForExprTest, AcceptASTVisitor) {
    MockASTVisitor mockVisitor;

    EXPECT_CALL(mockVisitor, visitForExpr(testing::Ref(*expr)))
        .Times(1);

    expr->accept(mockVisitor); 
}

TEST_F(ForExprTest, VisitIfExpr) {
    MockValueVisitor mockVisitor;

    EXPECT_CALL(mockVisitor, visitForExpr(testing::Ref(*expr)))
        .WillOnce(testing::Return(value));

    llvm::Value* result = expr->accept(mockVisitor);
    EXPECT_EQ(result, value);
}