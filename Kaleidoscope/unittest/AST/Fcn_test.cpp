#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <vector>
#include "AST/Fcn.hpp"

class MockExpr : public Expr {
public:
    MOCK_METHOD(const std::string, getType, (), (const, override));
    MOCK_METHOD(std::string, toString, (), (const, override));
};

TEST(FcnPrototypeTest, ConstructorAndGetName) {
    std::vector<std::string> args = {"x", "y", "z"};
    FcnPrototype proto("myFunc", args);
    EXPECT_EQ(proto.getName(), "myFunc");
}

TEST(FcnPrototypeTest, GetNameReturnsReference) {
    std::vector<std::string> args = {};
    FcnPrototype proto("foo", args);
    const std::string& nameRef = proto.getName();
    EXPECT_EQ(nameRef, "foo");
}

TEST(FcnTest, ConstructorStoresPrototypeAndBody) {
    auto proto = std::make_unique<FcnPrototype>("bar", std::vector<std::string>{"a"});
    auto body = std::make_unique<MockExpr>();
    auto bodyPtr = body.get();

    Fcn fcn(std::move(proto), std::move(body));
    EXPECT_EQ(fcn.getName(), "bar");
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