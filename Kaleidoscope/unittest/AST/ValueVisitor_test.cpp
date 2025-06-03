#include "gtest/gtest.h"

#include "AST/Expr.hpp"
#include "AST/Fcn.hpp"
#include "AST/ValueVisitor.hpp"
#include "AST/PrototypeRegistry.hpp"

using namespace llvm;

// Test fixture for CodegenVisitor
class CodegenVisitorTest : public ::testing::Test {
protected:
    LLVMContext context;
    std::unique_ptr<Module> module;
    std::unique_ptr<IRBuilder<>> builder;
    std::unique_ptr<CodegenVisitor> visitor;

    void SetUp() override {
        module = std::make_unique<llvm::Module>("test_module", context);
        PrototypeRegistry::get()->setModule(module.get());
        builder = std::make_unique<llvm::IRBuilder<>>(context);

        visitor = std::make_unique<CodegenVisitor>(&context, module.get(), builder.get());
    }

    void TearDown() override {
        PrototypeRegistry::reset();
    }

    void addPrototypeToRegistry() {
        // Add a prototype with two args to the registry
        std::vector<std::string> args = {"a", "b"};
        auto proto = std::make_unique<FcnPrototype>("foo", args);
        PrototypeRegistry::addFcnPrototype("foo", std::move(proto));
    }

    BinaryExpr makeBinaryExpr(char op) {
        auto lhs = std::make_unique<NumberExpr>(1.0);
        auto rhs = std::make_unique<NumberExpr>(2.0);
        return BinaryExpr(op, std::move(lhs), std::move(rhs));
    }
};

TEST_F(CodegenVisitorTest, VisitNumberExprReturnsDouble) {
    NumberExpr expr(42.0);
    Value* actVal = visitor->visitNumberExpr(expr);
    Value* expVal = ConstantFP::get(context, APFloat(3.14));
    
    ASSERT_NE(actVal, nullptr);
    EXPECT_TRUE(actVal->getType()->isDoubleTy());
}

TEST_F(CodegenVisitorTest, VisitVariableExprReturnsCorrectVal) {
    llvm::Value* dummy = ConstantFP::get(context, APFloat(3.14));
    visitor->setNamedValue("x", dummy);

    VariableExpr expr("x");
    llvm::Value* val = visitor->visitVariableExpr(expr);
    EXPECT_EQ(val, dummy);
}

TEST_F(CodegenVisitorTest, VisitForNonExistentVarReturnsNull) {
    VariableExpr expr("x");
    Value* actVal = visitor->visitVariableExpr(expr);
    EXPECT_EQ(actVal, nullptr);
}

TEST_F(CodegenVisitorTest, VisitBinaryExprAdd) {
    BinaryExpr expr = makeBinaryExpr('+');
    llvm::Value* val = visitor->visitBinaryExpr(expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(val->getType()->isDoubleTy());
}

TEST_F(CodegenVisitorTest, VisitBinaryExprMinus) {
    BinaryExpr expr = makeBinaryExpr('-');
    llvm::Value* val = visitor->visitBinaryExpr(expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(val->getType()->isDoubleTy());
}

TEST_F(CodegenVisitorTest, VisitBinaryExprTimes) {
    BinaryExpr expr = makeBinaryExpr('*');
    llvm::Value* val = visitor->visitBinaryExpr(expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(val->getType()->isDoubleTy());
}

TEST_F(CodegenVisitorTest, VisitBinaryExprLT) {
    BinaryExpr expr = makeBinaryExpr('<');
    llvm::Value* val = visitor->visitBinaryExpr(expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(val->getType()->isDoubleTy());
}

TEST_F(CodegenVisitorTest, VisitBinaryExprNonOp) {
    BinaryExpr expr = makeBinaryExpr('a');
    llvm::Value* val = visitor->visitBinaryExpr(expr);
    EXPECT_EQ(val, nullptr);
}

TEST_F(CodegenVisitorTest, VisitCallExpr) {
    addPrototypeToRegistry();
    std::vector<std::unique_ptr<Expr>> callArgs;
    callArgs.push_back(std::make_unique<NumberExpr>(5.0));
    callArgs.push_back(std::make_unique<NumberExpr>(7.0));
    CallExpr callExpr("foo", std::move(callArgs));

    llvm::Value* val = visitor->visitCallExpr(callExpr);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(val->getName(), "calltmp");
}

TEST_F(CodegenVisitorTest, VisitCallExprProtoNotRegistered) {
    std::vector<std::unique_ptr<Expr>> callArgs;
    callArgs.push_back(std::make_unique<NumberExpr>(5.0));
    callArgs.push_back(std::make_unique<NumberExpr>(7.0));
    CallExpr callExpr("foo", std::move(callArgs));

    llvm::Value* val = visitor->visitCallExpr(callExpr);
    EXPECT_EQ(val, nullptr);
}

TEST_F(CodegenVisitorTest, VisitCallExprWrongNumberArgs) {
    std::vector<std::unique_ptr<Expr>> callArgs;
    callArgs.push_back(std::make_unique<NumberExpr>(7.0));
    CallExpr callExpr("foo", std::move(callArgs));

    llvm::Value* val = visitor->visitCallExpr(callExpr);
    EXPECT_EQ(val, nullptr);
}

TEST_F(CodegenVisitorTest, VisitCallExprBadArg) {
    std::vector<std::unique_ptr<Expr>> callArgs;
    callArgs.push_back(std::make_unique<NumberExpr>(7.0));
    callArgs.push_back(std::make_unique<BinaryExpr>(makeBinaryExpr('a')));
    CallExpr callExpr("foo", std::move(callArgs));
    llvm::Value* val = visitor->visitCallExpr(callExpr);
    EXPECT_EQ(val, nullptr);
}

TEST_F(CodegenVisitorTest, VisitIfExpr) {
    IfExpr expr(std::make_unique<BinaryExpr>(makeBinaryExpr('<')), std::make_unique<NumberExpr>(1), std::make_unique<NumberExpr>(2));
    llvm::Value* val = visitor->visitIfExpr(expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(llvm::isa<llvm::PHINode>(val));
}

TEST_F(CodegenVisitorTest, VisitIfExprBadCond) {
    IfExpr expr(std::make_unique<VariableExpr>("@"), std::make_unique<NumberExpr>(1), std::make_unique<NumberExpr>(2));
    llvm::Value* val = visitor->visitIfExpr(expr);
    ASSERT_EQ(val, nullptr);
}

TEST_F(CodegenVisitorTest, VisitIfExprBadThen) {
    IfExpr expr(std::make_unique<NumberExpr>(1), std::make_unique<VariableExpr>("@"), std::make_unique<NumberExpr>(2));
    llvm::Value* val = visitor->visitIfExpr(expr);
    ASSERT_EQ(val, nullptr);
}

TEST_F(CodegenVisitorTest, VisitIfExprBadElse) {
    IfExpr expr(std::make_unique<NumberExpr>(1), std::make_unique<NumberExpr>(2), std::make_unique<VariableExpr>("@"));
    llvm::Value* val = visitor->visitIfExpr(expr);
    ASSERT_EQ(val, nullptr);
}


TEST_F(CodegenVisitorTest, VisitFcnPrototypeCreatesFunction) {
    std::vector<std::string> args = {"x", "y"};
    FcnPrototype proto("bar", args);
    llvm::Value* val = visitor->visitFcnPrototype(proto);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(llvm::isa<llvm::Function>(val));
}

// TODO: Get this working, the FPM isn't working in the test
// environment
TEST_F(CodegenVisitorTest, DISABLED_VisitFcnCreatesFunction) {
    std::vector<std::string> args = {"x", "y"};
    auto proto = std::make_unique<FcnPrototype>("baz", args);
    auto body = std::make_unique<BinaryExpr>(makeBinaryExpr('+'));
    Fcn fcn(std::move(proto), std::move(body));
    llvm::Value* val = visitor->visitFcn(fcn);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(llvm::isa<llvm::Function>(val));
}


TEST_F(CodegenVisitorTest, VisitFcnBadBody) {
    std::vector<std::string> args = {"x", "y"};
    auto proto = std::make_unique<FcnPrototype>("baz", args);
    auto body = std::make_unique<BinaryExpr>(makeBinaryExpr('a'));
    Fcn fcn(std::move(proto), std::move(body));
    llvm::Value* val = visitor->visitFcn(fcn);
    // EXPECT_EQ(val, nullptr);
    auto function = static_cast<Function*>(val);
    EXPECT_FALSE(function->getParent())
        << "Function should be removed from its parent";
}