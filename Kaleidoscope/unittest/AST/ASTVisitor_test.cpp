#include "gtest/gtest.h"

#include "AST/Expr.hpp"
#include "AST/Fcn.hpp"
#include "AST/ASTVisitor.hpp"
#include "mocks/AST/MockASTVisitor.hpp"

TEST(AcceptOnNodeTest, VisitNumberExpr) {
    MockASTVisitor mockVisitor;
    NumberExpr expr(3.14);

    EXPECT_CALL(mockVisitor, visitNumberExpr(testing::Ref(expr)))
        .Times(1);

    expr.accept(mockVisitor);
}

TEST(AcceptOnNodeTest, VisitVariableExpr) {
    MockASTVisitor mockVisitor;
    VariableExpr expr("x");

    EXPECT_CALL(mockVisitor, visitVariableExpr(testing::Ref(expr)))
        .Times(1);

    expr.accept(mockVisitor);
}

TEST(AcceptOnNodeTest, VisitBinaryExpr) {
    MockASTVisitor mockVisitor;
    auto lhs = std::make_unique<NumberExpr>(1.0);
    auto rhs = std::make_unique<NumberExpr>(2.0);
    BinaryExpr expr('+', std::move(lhs), std::move(rhs));

    EXPECT_CALL(mockVisitor, visitBinaryExpr(testing::Ref(expr)))
        .Times(1);

    expr.accept(mockVisitor);
}

TEST(AcceptOnNodeTest, VisitCallExpr) {
    MockASTVisitor mockVisitor;
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<NumberExpr>(1.0));
    CallExpr expr("foo", std::move(args));

    EXPECT_CALL(mockVisitor, visitCallExpr(testing::Ref(expr)))
        .Times(1);

    expr.accept(mockVisitor);
}

TEST(AcceptOnNodeTest, VisitFcnPrototype) {
    MockASTVisitor mockVisitor;
    std::vector<std::string> args = {"x", "y"};
    FcnPrototype proto("myFunc", args);

    EXPECT_CALL(mockVisitor, visitFcnPrototype(testing::Ref(proto)))
        .Times(1);

    proto.accept(mockVisitor);
}

TEST(AcceptOnNodeTest, VisitFcn) {
    MockASTVisitor mockVisitor;
    auto proto = std::make_unique<FcnPrototype>("myFunc", std::vector<std::string>{"x", "y"});
    auto body = std::make_unique<NumberExpr>(42.0);
    Fcn fcn(std::move(proto), std::move(body));

    EXPECT_CALL(mockVisitor, visitFcn(testing::Ref(fcn)))
        .Times(1);

    fcn.accept(mockVisitor);
}
