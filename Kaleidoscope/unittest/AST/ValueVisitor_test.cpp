#include "gtest/gtest.h"

#include "AST/Expr.hpp"
#include "AST/Fcn.hpp"
#include "AST/Precedence.hpp"
#include "AST/PrototypeRegistry.hpp"
#include "AST/ValueVisitor.hpp"

using namespace llvm;

// Test fixture for CodegenVisitor
class CodegenVisitorTest : public ::testing::Test {
protected:
    LLVMContext context;
    std::unique_ptr<Module> module;
    std::unique_ptr<IRBuilder<>> builder;
    std::unique_ptr<CodegenVisitor> visitor;

    void SetUp() override {
        module = std::make_unique<Module>("test_module", context);
        PrototypeRegistry::get()->setModule(module.get());
        builder = std::make_unique<IRBuilder<>>(context);

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
    Value* dummy = ConstantFP::get(context, APFloat(3.14));
    visitor->setNamedValue("x", dummy);

    VariableExpr expr("x");
    Value* val = visitor->visitVariableExpr(expr);
    EXPECT_EQ(val, dummy);
}

TEST_F(CodegenVisitorTest, VisitForNonExistentVarReturnsNull) {
    VariableExpr expr("x");
    Value* actVal = visitor->visitVariableExpr(expr);
    EXPECT_EQ(actVal, nullptr);
}

TEST_F(CodegenVisitorTest, VisitBinaryExprAdd) {
    BinaryExpr expr = makeBinaryExpr('+');
    Value* val = visitor->visitBinaryExpr(expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(val->getType()->isDoubleTy());
}

TEST_F(CodegenVisitorTest, VisitBinaryExprMinus) {
    BinaryExpr expr = makeBinaryExpr('-');
    Value* val = visitor->visitBinaryExpr(expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(val->getType()->isDoubleTy());
}

TEST_F(CodegenVisitorTest, VisitBinaryExprTimes) {
    BinaryExpr expr = makeBinaryExpr('*');
    Value* val = visitor->visitBinaryExpr(expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(val->getType()->isDoubleTy());
}

TEST_F(CodegenVisitorTest, VisitBinaryExprLT) {
    BinaryExpr expr = makeBinaryExpr('<');
    Value* val = visitor->visitBinaryExpr(expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(val->getType()->isDoubleTy());
}

TEST_F(CodegenVisitorTest, VisitBinaryExprNonOp) {
    BinaryExpr expr = makeBinaryExpr('a');
    EXPECT_DEATH(visitor->visitBinaryExpr(expr), "binary operator not found");
}

TEST_F(CodegenVisitorTest, VisitCustomBinaryExpr) {
    std::vector<std::string> args = {"x", "y"};
    auto proto = std::make_unique<FcnPrototype>("binary`", args, true);
    PrototypeRegistry::addFcnPrototype("binary`", std::move(proto));

    BinaryExpr expr = makeBinaryExpr('`');
    Value* val = visitor->visitBinaryExpr(expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(val->getType()->isDoubleTy());
}

// For unary operators, there are no predefined ops
TEST_F(CodegenVisitorTest, VisitUnaryExprNonOp) {
    UnaryExpr expr = UnaryExpr('`', std::make_unique<NumberExpr>(7));
    EXPECT_DEATH(visitor->visitUnaryExpr(expr), "unary operator not found");
}

TEST_F(CodegenVisitorTest, VisitCustomUnaryOperator) {
    std::vector<std::string> args = {"x"};
    auto proto = std::make_unique<FcnPrototype>("unary`", args, true);
    PrototypeRegistry::addFcnPrototype("unary`", std::move(proto));

    UnaryExpr expr = UnaryExpr('`', std::make_unique<NumberExpr>(7));
    Value* val = visitor->visitUnaryExpr(expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(val->getType()->isDoubleTy());
}

TEST_F(CodegenVisitorTest, VisitCustomUnaryOperatorBadBody) {
    std::vector<std::string> args = {"x"};
    auto proto = std::make_unique<FcnPrototype>("unary`", args, true);
    PrototypeRegistry::addFcnPrototype("unary`", std::move(proto));

    UnaryExpr expr = UnaryExpr('`', std::make_unique<VariableExpr>("a"));
    Value* val = visitor->visitUnaryExpr(expr);
    EXPECT_FALSE(val);
}

TEST_F(CodegenVisitorTest, VisitCallExpr) {
    addPrototypeToRegistry();
    std::vector<std::unique_ptr<Expr>> callArgs;
    callArgs.push_back(std::make_unique<NumberExpr>(5.0));
    callArgs.push_back(std::make_unique<NumberExpr>(7.0));
    CallExpr callExpr("foo", std::move(callArgs));

    Value* val = visitor->visitCallExpr(callExpr);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(val->getName(), "calltmp");
}

TEST_F(CodegenVisitorTest, VisitCallExprProtoNotRegistered) {
    std::vector<std::unique_ptr<Expr>> callArgs;
    callArgs.push_back(std::make_unique<NumberExpr>(5.0));
    callArgs.push_back(std::make_unique<NumberExpr>(7.0));
    CallExpr callExpr("foo", std::move(callArgs));

    Value* val = visitor->visitCallExpr(callExpr);
    EXPECT_EQ(val, nullptr);
}

TEST_F(CodegenVisitorTest, VisitCallExprWrongNumberArgs) {
    std::vector<std::unique_ptr<Expr>> callArgs;
    callArgs.push_back(std::make_unique<NumberExpr>(7.0));
    CallExpr callExpr("foo", std::move(callArgs));

    Value* val = visitor->visitCallExpr(callExpr);
    EXPECT_EQ(val, nullptr);
}

TEST_F(CodegenVisitorTest, VisitCallExprBadArg) {
    std::vector<std::unique_ptr<Expr>> callArgs;
    callArgs.push_back(std::make_unique<NumberExpr>(7.0));
    callArgs.push_back(std::make_unique<BinaryExpr>(makeBinaryExpr('a')));
    CallExpr callExpr("foo", std::move(callArgs));
    Value* val = visitor->visitCallExpr(callExpr);
    EXPECT_EQ(val, nullptr);
}

// TODO: Setup JIT for testing
TEST_F(CodegenVisitorTest, DISABLED_VisitIfExpr) {
    IfExpr expr(std::make_unique<BinaryExpr>(makeBinaryExpr('<')), std::make_unique<NumberExpr>(1), std::make_unique<NumberExpr>(2));
    Value* val = visitor->visitIfExpr(expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(isa<PHINode>(val));
}

TEST_F(CodegenVisitorTest, VisitIfExprBadCond) {
    IfExpr expr(std::make_unique<VariableExpr>("@"), std::make_unique<NumberExpr>(1), std::make_unique<NumberExpr>(2));
    Value* val = visitor->visitIfExpr(expr);
    ASSERT_EQ(val, nullptr);
}

TEST_F(CodegenVisitorTest, DISABLED_VisitIfExprBadThen) {
    IfExpr expr(std::make_unique<NumberExpr>(1), std::make_unique<VariableExpr>("@"), std::make_unique<NumberExpr>(2));
    Value* val = visitor->visitIfExpr(expr);
    ASSERT_EQ(val, nullptr);
}

TEST_F(CodegenVisitorTest, DISABLED_VisitIfExprBadElse) {
    IfExpr expr(std::make_unique<NumberExpr>(1), std::make_unique<NumberExpr>(2), std::make_unique<VariableExpr>("@"));
    Value* val = visitor->visitIfExpr(expr);
    ASSERT_EQ(val, nullptr);
}

// TODO: Dependent on JIT
TEST_F(CodegenVisitorTest, DISABLED_VisitForExpr) {
    std::string loopId = "i";
    auto Start = std::make_unique<NumberExpr>(0);
    auto End = std::make_unique<NumberExpr>(10);
    auto Step = std::make_unique<NumberExpr>(1);
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<NumberExpr>(1.0));
    auto Body = std::make_unique<CallExpr>("foo", std::move(args));
    auto expr = std::make_unique<ForExpr>(loopId, std::move(Start), std::move(End),
            std::move(Step), std::move(Body));

    auto val = visitor->visitForExpr(*expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(isa<Constant>(val));
}

TEST_F(CodegenVisitorTest, DISABLED_VisitForExprNullStep) {
    std::string loopId = "i";
    auto Start = std::make_unique<NumberExpr>(0);
    auto End = std::make_unique<NumberExpr>(10);
    // Step is optional, should codegen fine
    auto Step = nullptr;
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<NumberExpr>(1.0));
    auto Body = std::make_unique<CallExpr>("foo", std::move(args));
    auto expr = std::make_unique<ForExpr>(loopId, std::move(Start), std::move(End),
            std::move(Step), std::move(Body));

    auto val = visitor->visitForExpr(*expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(isa<Constant>(val));
}

TEST_F(CodegenVisitorTest, VisitForExprBadStart) {
    std::string loopId = "i";
    auto Start = std::make_unique<VariableExpr>("j");
    auto End = std::make_unique<NumberExpr>(10);
    auto Step = std::make_unique<NumberExpr>(1);
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<NumberExpr>(1.0));
    auto Body = std::make_unique<CallExpr>("foo", std::move(args));
    auto expr = std::make_unique<ForExpr>(loopId, std::move(Start), std::move(End),
            std::move(Step), std::move(Body));

    auto val = visitor->visitForExpr(*expr);
    ASSERT_EQ(val, nullptr);
}

TEST_F(CodegenVisitorTest, DISABLED_VisitForExprBadEnd) {
    std::string loopId = "i";
    auto Start = std::make_unique<NumberExpr>(0);
    auto End = std::make_unique<VariableExpr>("j");
    auto Step = std::make_unique<NumberExpr>(1);
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<NumberExpr>(1.0));
    auto Body = std::make_unique<CallExpr>("foo", std::move(args));
    auto expr = std::make_unique<ForExpr>(loopId, std::move(Start), std::move(End),
            std::move(Step), std::move(Body));

    auto val = visitor->visitForExpr(*expr);
    ASSERT_EQ(val, nullptr);
}

TEST_F(CodegenVisitorTest, DISABLED_VisitForExprBadStep) {
    std::string loopId = "i";
    auto Start = std::make_unique<NumberExpr>(0);
    auto End = std::make_unique<NumberExpr>(10);
    auto Step = std::make_unique<VariableExpr>("j");
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<NumberExpr>(1.0));
    auto Body = std::make_unique<CallExpr>("foo", std::move(args));
    auto expr = std::make_unique<ForExpr>(loopId, std::move(Start), std::move(End),
            std::move(Step), std::move(Body));

    auto val = visitor->visitForExpr(*expr);
    ASSERT_EQ(val, nullptr);
}

TEST_F(CodegenVisitorTest, DISABLED_VisitForExprBadBody) {
    std::string loopId = "i";
    auto Start = std::make_unique<NumberExpr>(0);
    auto End = std::make_unique<NumberExpr>(10);
    auto Step = std::make_unique<NumberExpr>(1);
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<NumberExpr>(1.0));
    auto Body = std::make_unique<VariableExpr>("j");
    auto expr = std::make_unique<ForExpr>(loopId, std::move(Start), std::move(End),
            std::move(Step), std::move(Body));

    auto val = visitor->visitForExpr(*expr);
    ASSERT_EQ(val, nullptr);
}

TEST_F(CodegenVisitorTest, VisitFcnPrototypeCreatesFunction) {
    std::vector<std::string> args = {"x", "y"};
    FcnPrototype proto("bar", args);
    Value* val = visitor->visitFcnPrototype(proto);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(isa<Function>(val));
}

// TODO: Get this working, the FPM isn't working in the test
// environment
TEST_F(CodegenVisitorTest, DISABLED_VisitFcnCreatesFunction) {
    std::vector<std::string> args = {"x", "y"};
    auto proto = std::make_unique<FcnPrototype>("baz", args);
    auto body = std::make_unique<BinaryExpr>(makeBinaryExpr('+'));
    Fcn fcn(std::move(proto), std::move(body));
    Value* val = visitor->visitFcn(fcn);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(isa<Function>(val));
}

TEST_F(CodegenVisitorTest, VisitFcnBadBody) {
    std::vector<std::string> args = {"x", "y"};
    auto proto = std::make_unique<FcnPrototype>("baz", args);
    auto body = std::make_unique<VariableExpr>("a");
    Fcn fcn(std::move(proto), std::move(body));
    Value* val = visitor->visitFcn(fcn);
    // EXPECT_EQ(val, nullptr);
    auto function = static_cast<Function*>(val);
    EXPECT_FALSE(function->getParent())
        << "Function should be removed from its parent";
}

TEST_F(CodegenVisitorTest, VisitBinaryOpFcnSetsPrecedence) {
    std::vector<std::string> args = {"x", "y"};
    auto proto = std::make_unique<FcnPrototype>("binary`", args, true, 17);
    // Use bad body on purpose to bail out and check precedence table
    auto body = std::make_unique<VariableExpr>("a");
    Fcn fcn(std::move(proto), std::move(body));
    Value* val = visitor->visitFcn(fcn);
    
    EXPECT_EQ(BIN_OP_PRECEDENCE['`'], 17);
    BIN_OP_PRECEDENCE.erase('`');
}