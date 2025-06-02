#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <vector>
#include "AST/Fcn.hpp"
#include "mocks/AST/MockExpr.hpp"
#include "mocks/AST/MockASTVisitor.hpp"
#include "mocks/AST/MockValueVisitor.hpp"

TEST(FcnPrototypeTest, ConstructorAndGetType) {
    std::vector<std::string> args = {"x", "y", "z"};
    FcnPrototype proto("myFunc", args);
    EXPECT_EQ(proto.getType(), "FunctionPrototype");
}

TEST(FcnPrototypeTest, GetArgsReturnsReference) {
    std::vector<std::string> args = {"arg1", "arg2"};
    FcnPrototype proto("testFunc", args);
    const std::vector<std::string>& argsRef = proto.getArgs();
    EXPECT_EQ(argsRef.size(), 2);
    EXPECT_EQ(argsRef[0], "arg1");
    EXPECT_EQ(argsRef[1], "arg2");
}

TEST(FcnPrototypeTest, GetNameReturnsReference) {
    std::vector<std::string> args = {};
    FcnPrototype proto("foo", args);
    const std::string& nameRef = proto.getName();
    EXPECT_EQ(nameRef, "foo");
}

TEST(FcnPrototypeTest, AcceptASTVisitor) {
    MockASTVisitor mockVisitor;
    std::vector<std::string> args = {"x", "y"};
    FcnPrototype proto("myFunc", args);

    EXPECT_CALL(mockVisitor, visitFcnPrototype(testing::Ref(proto)))
        .Times(1);

    proto.accept(mockVisitor);
}

TEST_F(MockedValueVisitorTest, VisitFcnPrototype) {
    MockValueVisitor mockVisitor;
    std::vector<std::string> args = {"x", "y"};
    FcnPrototype proto("myFunc", args);

    EXPECT_CALL(mockVisitor, visitFcnPrototype(testing::Ref(proto)))
        .WillOnce(testing::Return(value));

    llvm::Value* result = proto.accept(mockVisitor);
    EXPECT_EQ(result, value);
}


TEST(FcnTest, ConstructorAndGetType) {
    auto proto = std::make_unique<FcnPrototype>("bar", std::vector<std::string>{"a"});
    auto body = std::make_unique<MockExpr>();

    Fcn fcn(std::move(proto), std::move(body));
    EXPECT_EQ(fcn.getType(), "Function");
}


TEST(FcnTest, GetNameReturnsCorrectName) {
    auto proto = std::make_unique<FcnPrototype>("bar", std::vector<std::string>{"a"});
    auto body = std::make_unique<MockExpr>();

    Fcn fcn(std::move(proto), std::move(body));
    EXPECT_EQ(fcn.getName(), "bar");
}

TEST(FcnTest, GetBodyReturnsCorrectBody) {
    auto proto = std::make_unique<FcnPrototype>("bar", std::vector<std::string>{"a"});
    auto body = std::make_unique<MockExpr>();
    auto bodyPtr = body.get();

    Fcn fcn(std::move(proto), std::move(body));
    EXPECT_EQ(fcn.getBody(), bodyPtr);
}

TEST(FcnTest, ConstructorWithNullPrototypeAndBody) {
    std::unique_ptr<FcnPrototype> proto = nullptr;
    std::unique_ptr<Expr> body = nullptr;
    Fcn fcn(std::move(proto), std::move(body));
    
    EXPECT_EQ(fcn.getName(), "");
    EXPECT_EQ(fcn.getPrototype(), nullptr);
    EXPECT_EQ(fcn.getBody(), nullptr);
}

TEST(FcnTest, AcceptASTVisitor) {
    MockASTVisitor mockVisitor;
    auto proto = std::make_unique<FcnPrototype>("myFunc", std::vector<std::string>{"x", "y"});
    auto body = std::make_unique<NumberExpr>(42.0);
    Fcn fcn(std::move(proto), std::move(body));

    EXPECT_CALL(mockVisitor, visitFcn(testing::Ref(fcn)))
        .Times(1);

    fcn.accept(mockVisitor);
}

TEST_F(MockedValueVisitorTest, VisitFcn) {
    MockValueVisitor mockVisitor;
    auto proto = std::make_unique<FcnPrototype>("myFunc", std::vector<std::string>{"x", "y"});
    auto body = std::make_unique<NumberExpr>(42.0);
    Fcn fcn(std::move(proto), std::move(body));

    EXPECT_CALL(mockVisitor, visitFcn(testing::Ref(fcn)))
        .WillOnce(testing::Return(value));

    llvm::Value* result = fcn.accept(mockVisitor);
    EXPECT_EQ(result, value);
}