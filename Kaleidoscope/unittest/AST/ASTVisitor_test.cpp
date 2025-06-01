#include "gtest/gtest.h"

#include "AST/Expr.hpp"
#include "AST/Fcn.hpp"
#include "AST/ASTVisitor.hpp"
#include "mocks/AST/MockASTVisitor.hpp"

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
