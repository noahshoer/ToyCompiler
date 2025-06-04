
#include "llvm/IR/Verifier.h"

#include "AST/Expr.hpp"
#include "AST/Fcn.hpp"
#include "AST/Precedence.hpp"
#include "AST/PrototypeRegistry.hpp"
#include "AST/ValueVisitor.hpp"

llvm::Value* CodegenVisitor::visitNumberExpr(NumberExpr &expr) {
    // Constants are uniqued and shared, so we use get() to get/create a constant
    llvm::Value* value = llvm::ConstantFP::get(*context, llvm::APFloat(expr.getValue()));
    return value;
}

llvm::Value* CodegenVisitor::visitVariableExpr(VariableExpr &expr) {
    llvm::Value* value = namedValues[expr.getName()];
    if (!value) {
        logError("Variable '" + expr.getName() + "' is unknown");
    }
    return value;
}

llvm::Value* CodegenVisitor::visitBinaryExpr(BinaryExpr &expr) {
    // Handle code generation for BinaryExpr
    auto lhs = expr.getLHS()->accept(*this);
    auto rhs = expr.getRHS()->accept(*this);
    if (!lhs || !rhs) { // base case of recursion
        return nullptr;
    }

    // The names passed to the builder methods are just meant to be hints
    // for what the operation is
    switch (expr.getOp()) {
        case '+':
            return builder->CreateFAdd(lhs, rhs, "addtmp");
        case '-':
            return builder->CreateFSub(lhs, rhs, "subtmp");
        case '*':
            return builder->CreateFMul(lhs, rhs, "multmp");
        case '<':
            lhs = builder->CreateFCmpULT(lhs, rhs, "cmptmp");
            // LLVM fcmp always returns a one bit integer, so we need to convert it to double
            // as that is (at this moment) what Kaleidoscope uses for all its values
            return builder->CreateUIToFP(lhs, llvm::Type::getDoubleTy(*context), "booltmp");
        default:
            break;
    }

    // If not builtin, it is a custom op
    llvm::Function* f = PrototypeRegistry::getFunction(std::string("binary") + expr.getOp(), *this);
    assert(f && "binary operator not found!");

    llvm::Value* ops[2] = {lhs, rhs};
    return builder->CreateCall(f, ops, "binop");
}

llvm::Value* CodegenVisitor::visitUnaryExpr(UnaryExpr &expr) {
    llvm::Value* operand = expr.getOperand()->accept(*this);
    if (!operand) {
        return nullptr;
    }

    llvm::Function* f = PrototypeRegistry::getFunction(std::string("unary") + expr.getOp(), *this);
    assert(f && "unary operator not found!");

    return builder->CreateCall(f, operand, "unop");
}

llvm::Value* CodegenVisitor::visitCallExpr(CallExpr &expr) {
    llvm::Function* callee = PrototypeRegistry::getFunction(expr.getCalleeName(), *this);
    if (!callee) {
        logError("Unknown function called: " + expr.getCalleeName());
        return nullptr;
    }

    if (callee->arg_size() != expr.getNumArgs()) {
        logError("Incorrect number of arguments passed to function: " + expr.getCalleeName());
        return nullptr;
    }

    std::vector<llvm::Value*> args;
    for (const auto &arg : expr.getArgs()) {
        args.push_back(arg->accept(*this));
        if (!args.back()) {
            return nullptr;
        }
    }

    return builder->CreateCall(callee, args, "calltmp");
}

llvm::Value* CodegenVisitor::visitIfExpr(IfExpr &expr) {
    llvm::Value* condValue = expr.getCond()->accept(*this);
    if (!condValue) {
        return nullptr;
    }

    // Get the one bit bool directly
    condValue = builder->CreateFCmpONE(condValue, 
        llvm::ConstantFP::get(*context, llvm::APFloat(0.0)), "ifcond");

    llvm::Function* function = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBB =
        llvm::BasicBlock::Create(*context, "then", function);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(*context, "else");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context, "ifcont");

    builder->CreateCondBr(condValue, thenBB, elseBB);

    builder->SetInsertPoint(thenBB);
    llvm::Value* thenValue = expr.getThen()->accept(*this);
    if (!thenValue) {
        return nullptr;
    }
    builder->CreateBr(mergeBB);
    // Get the insert block for the phi to protect against thenBB 
    // changing the emittee block during recursive codegen
    thenBB = builder->GetInsertBlock();

    // Add the else block to the function
    function->insert(function->end(), elseBB);
    builder->SetInsertPoint(elseBB);
    llvm::Value* elseValue = expr.getElse()->accept(*this);
    if (!elseValue) {
        return nullptr;
    }

    builder->CreateBr(mergeBB);
    elseBB = builder->GetInsertBlock();

    function->insert(function->end(), mergeBB);
    builder->SetInsertPoint(mergeBB);
    llvm::PHINode* pn = 
        builder->CreatePHI(llvm::Type::getDoubleTy(*context), 2, "iftmp");

    pn->addIncoming(thenValue, thenBB);
    pn->addIncoming(elseValue, elseBB);
    return pn;
}

llvm::Value* CodegenVisitor::visitForExpr(ForExpr &expr) {
    // Emit the start code, variable is not in scope
    llvm::Value* startVal = expr.getStart()->accept(*this);
    if (!startVal) {
        return nullptr;
    }

    // Insert loop header block after the current block
    llvm::Function* function = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock* preHeaderBB = builder->GetInsertBlock();
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(*context, "loop", function);

    // Explicitly add a fal through from the current block
    builder->CreateBr(loopBB);

    builder->SetInsertPoint(loopBB);
    llvm::PHINode* var = builder->CreatePHI(llvm::Type::getDoubleTy(*context),
                                            2, expr.getVarName());
    var->addIncoming(startVal, preHeaderBB);

    // Shadow the var if it exists
    llvm::Value* oldVal = namedValues[expr.getVarName()];
    setNamedValue(expr.getVarName(), var);

    // Emit the body, ignoring the computed value
    if (!expr.getBody()->accept(*this)) {
        return nullptr;
    }

    // Emit the step value, with 1.0 as the default
    llvm::Value* stepVal = nullptr;
    if (expr.getStep()) {
        stepVal = expr.getStep()->accept(*this);
        if (!stepVal) {
            return nullptr;
        }
    } else {
        stepVal = llvm::ConstantFP::get(*context, llvm::APFloat(1.0));
    }
    llvm::Value* nextVar = builder->CreateFAdd(var, stepVal, "nextvar");

    // Compute the end condition
    llvm::Value* endCond = expr.getEnd()->accept(*this);
    if (!endCond) {
        return nullptr;
    }
    endCond = builder->CreateFCmpONE(endCond, llvm::ConstantFP::get(*context, 
                                    llvm::APFloat(0.0)), "loopcond");

    // Evaluate to check for exit
    llvm::BasicBlock* loopEndBB = builder->GetInsertBlock();
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(*context, "afterloop", function);
    builder->CreateCondBr(endCond, loopBB, afterBB);
    builder->SetInsertPoint(afterBB);

    // New PHI entry for back edge
    var->addIncoming(nextVar, loopEndBB);
    // Restore unshadowed variable
    if (oldVal) {
        setNamedValue(expr.getVarName(), oldVal);
    } else {
        namedValues.erase(expr.getVarName());
    }

    // For expr always returns 0.0
    return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*context));
}

llvm::Value* CodegenVisitor::visitFcnPrototype(FcnPrototype &proto) {
    // Create a function prototype in LLVM IR
    std::vector<llvm::Type*> argTypes(proto.getArgs().size(), llvm::Type::getDoubleTy(*context));
    llvm::FunctionType* fType = llvm::FunctionType::get(llvm::Type::getDoubleTy(*context), argTypes, false);
    llvm::Function* function = llvm::Function::Create(fType, llvm::Function::ExternalLinkage, proto.getName(), module);
    
    // Set argument names
    unsigned idx = 0;
    for (auto &arg : function->args()) {
        arg.setName(proto.getArgs()[idx++]);
    }
    
    return function;
}

llvm::Value* CodegenVisitor::visitFcn(Fcn &fcn) {
    auto protoName = fcn.getName();
    auto &p = *fcn.getPrototype();
    PrototypeRegistry::addFcnPrototype(protoName, std::move(fcn.releasePrototype()));
    llvm::Function* function = PrototypeRegistry::getFunction(protoName, *this);

    if (!function) {
        return nullptr;
    }

    if (p.isBinaryOp()) {
        BIN_OP_PRECEDENCE[p.getOperatorName()] = p.getBinaryPrecedence();
    }

    // Create a new basic block for the function body
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(*context, "entry", function);
    builder->SetInsertPoint(bb);

    namedValues.clear();
    for (auto &arg : function->args()) {
        // Map the argument names to their corresponding LLVM values
        setNamedValue(arg.getName().str(), &arg);
    }

    if (auto retVal = fcn.getBody()->accept(*this)) {
        // If the function body returns a value, create a return instruction
        builder->CreateRet(retVal);

        // Validates the generated code
        llvm::verifyFunction(*function);

        // Optimize the function
        fpm->run(*function, *fam);
        return function;
    }

    function->eraseFromParent(); // If the body is invalid, remove the function
    return function;
}