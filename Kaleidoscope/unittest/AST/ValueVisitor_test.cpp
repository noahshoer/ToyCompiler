#include "gtest/gtest.h"

#include "llvm/Passes/PassBuilder.h"

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

        // Setup a basic basic block
        auto funcType = FunctionType::get(Type::getVoidTy(context), 
                                            false);
        auto func = Function::Create(funcType, Function::ExternalLinkage, 
                                                    "dummy", module.get());
        auto BB = BasicBlock::Create(context, "entry", func);
        builder->SetInsertPoint(BB);
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

// TODO: Builder has no insert block
TEST_F(CodegenVisitorTest, VisitVariableExprReturnsCorrectVal) {
    auto allocaInst = visitor->createEntryBlockAlloca(
                                    builder->GetInsertBlock()->getParent(), "x");
    visitor->setNamedValue("x", allocaInst);

    VariableExpr expr("x");
    Value* val = visitor->visitVariableExpr(expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(llvm::isa<llvm::LoadInst>(val));
    EXPECT_EQ(*val->getName().data(), 'x');
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

TEST_F(CodegenVisitorTest, VisitBinaryAssignment) {
    auto allocaInst = visitor->createEntryBlockAlloca(
                                    builder->GetInsertBlock()->getParent(), "x");
    visitor->setNamedValue("x", allocaInst);

    BinaryExpr expr = BinaryExpr('=', std::make_unique<VariableExpr>("x"), 
                                        std::make_unique<NumberExpr>(1));
    Value* val = visitor->visitBinaryExpr(expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(val->getType()->isDoubleTy());
}

TEST_F(CodegenVisitorTest, VisitBinaryAssignmentNoVar) {
    BinaryExpr expr = BinaryExpr('=', std::make_unique<VariableExpr>("x"), 
                                        std::make_unique<NumberExpr>(1));
    Value* val = visitor->visitBinaryExpr(expr);
    EXPECT_FALSE(val);
}

TEST_F(CodegenVisitorTest, VisitBinaryAssignmentNotToVariableExpr) {
    BinaryExpr expr = BinaryExpr('=', std::make_unique<NumberExpr>(0), 
                                        std::make_unique<NumberExpr>(1));
    Value* val = visitor->visitBinaryExpr(expr);
    EXPECT_FALSE(val);
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

TEST_F(CodegenVisitorTest, VisitIfExpr) {
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

TEST_F(CodegenVisitorTest, VisitIfExprBadThen) {
    IfExpr expr(std::make_unique<NumberExpr>(1), std::make_unique<VariableExpr>("@"), std::make_unique<NumberExpr>(2));
    Value* val = visitor->visitIfExpr(expr);
    ASSERT_EQ(val, nullptr);
}

TEST_F(CodegenVisitorTest, VisitIfExprBadElse) {
    IfExpr expr(std::make_unique<NumberExpr>(1), std::make_unique<NumberExpr>(2), std::make_unique<VariableExpr>("@"));
    Value* val = visitor->visitIfExpr(expr);
    ASSERT_EQ(val, nullptr);
}

TEST_F(CodegenVisitorTest, VisitForExpr) {
    std::string loopId = "i";
    auto Start = std::make_unique<NumberExpr>(0);
    auto End = std::make_unique<NumberExpr>(10);
    auto Step = std::make_unique<NumberExpr>(1);
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<NumberExpr>(1.0));
    auto Body = std::make_unique<NumberExpr>(1);
    auto expr = std::make_unique<ForExpr>(loopId, std::move(Start), std::move(End),
            std::move(Step), std::move(Body));

    auto val = visitor->visitForExpr(*expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(isa<Constant>(val));
}

TEST_F(CodegenVisitorTest, VisitForExprNullStep) {
    std::string loopId = "i";
    auto Start = std::make_unique<NumberExpr>(0);
    auto End = std::make_unique<NumberExpr>(10);
    // Step is optional, should codegen fine
    auto Step = nullptr;
    std::vector<std::unique_ptr<Expr>> args;
    args.push_back(std::make_unique<NumberExpr>(1.0));
    auto Body = std::make_unique<NumberExpr>(1);
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

TEST_F(CodegenVisitorTest, VisitForExprBadEnd) {
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

TEST_F(CodegenVisitorTest, VisitForExprBadStep) {
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

TEST_F(CodegenVisitorTest, VisitForExprBadBody) {
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

TEST_F(CodegenVisitorTest, VisitVarExpr) {
    VarNameVector args;
    args.push_back(std::make_pair("x", std::make_unique<NumberExpr>(1)));
    VarExpr expr(std::move(args), std::make_unique<NumberExpr>(3));

    auto val = visitor->visitVarExpr(expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(isa<Constant>(val));
}

TEST_F(CodegenVisitorTest, VisitVarExprMultipleVars) {
    VarNameVector args;
    args.push_back(std::make_pair("x", std::make_unique<NumberExpr>(1)));
    args.push_back(std::make_pair("y", std::make_unique<NumberExpr>(2)));
    VarExpr expr(std::move(args), std::make_unique<NumberExpr>(3));

    auto val = visitor->visitVarExpr(expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(isa<Constant>(val));
}

TEST_F(CodegenVisitorTest, VisitVarExprNullInit) {
    VarNameVector args;
    args.push_back(std::make_pair("z", nullptr));
    VarExpr expr(std::move(args), std::make_unique<NumberExpr>(3));

    auto val = visitor->visitVarExpr(expr);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(isa<Constant>(val));
}

TEST_F(CodegenVisitorTest, VisitVarExprCantAssignItself) {
    VarNameVector args;
    args.push_back(std::make_pair("x", std::make_unique<VariableExpr>("x")));
    VarExpr expr(std::move(args), std::make_unique<NumberExpr>(3));

    auto val = visitor->visitVarExpr(expr);
    ASSERT_FALSE(val);
}

TEST_F(CodegenVisitorTest, VisitVarExprBadBody) {
    VarNameVector args;
    args.push_back(std::make_pair("x", std::make_unique<NumberExpr>(1)));
    VarExpr expr(std::move(args), std::make_unique<VariableExpr>("q"));

    auto val = visitor->visitVarExpr(expr);
    ASSERT_FALSE(val);
}

TEST_F(CodegenVisitorTest, VisitFcnPrototypeCreatesFunction) {
    std::vector<std::string> args = {"x", "y"};
    FcnPrototype proto("bar", args);
    Value* val = visitor->visitFcnPrototype(proto);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(isa<Function>(val));
}

// TODO: Figure out how to fake the debug file, or just add a debugging control
// so that these tests don't have to deal with it
TEST_F(CodegenVisitorTest, DISABLED_VisitFcnCreatesFunction) {
    std::vector<std::string> args = {"x", "y"};
    auto proto = std::make_unique<FcnPrototype>("baz", args);
    auto body = std::make_unique<BinaryExpr>(makeBinaryExpr('+'));
    Fcn fcn(std::move(proto), std::move(body));
    Value* val = visitor->visitFcn(fcn);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(isa<Function>(val));
}

TEST_F(CodegenVisitorTest, DISABLED_VisitFcnCreatesFunctionWithFPM) {
    auto fpm = std::make_unique<FunctionPassManager>();
    auto fam = std::make_unique<FunctionAnalysisManager>();

    PassBuilder pb;
    pb.registerFunctionAnalyses(*fam);

    visitor->setFAM(fam.get());
    visitor->setFPM(fpm.get());

    std::vector<std::string> args = {"x", "y"};
    auto proto = std::make_unique<FcnPrototype>("baz", args);
    auto body = std::make_unique<BinaryExpr>(makeBinaryExpr('+'));
    Fcn fcn(std::move(proto), std::move(body));
    Value* val = visitor->visitFcn(fcn);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(isa<Function>(val));
}

TEST_F(CodegenVisitorTest, DISABLED_VisitFcnBadBody) {
    std::vector<std::string> args = {"x", "y"};
    auto proto = std::make_unique<FcnPrototype>("baz", args);
    auto body = std::make_unique<VariableExpr>("a");
    Fcn fcn(std::move(proto), std::move(body));
    Value* val = visitor->visitFcn(fcn);
    EXPECT_EQ(val, nullptr);
}

TEST_F(CodegenVisitorTest, DISABLED_VisitBinaryOpFcnSetsPrecedence) {
    std::vector<std::string> args = {"x", "y"};
    auto proto = std::make_unique<FcnPrototype>("binary`", args, true, 17);
    // Use bad body on purpose to bail out and check precedence table
    auto body = std::make_unique<VariableExpr>("a");
    Fcn fcn(std::move(proto), std::move(body));
    Value* val = visitor->visitFcn(fcn);
    
    EXPECT_EQ(BIN_OP_PRECEDENCE['`'], 17);
    BIN_OP_PRECEDENCE.erase('`');
}