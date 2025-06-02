
#include "llvm/IR/Verifier.h"

#include "AST/Expr.hpp"
#include "AST/Fcn.hpp"
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
            logError("Invalid binary operator: " + std::string(1, expr.getOp()));
            return nullptr;
    }
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
    return nullptr;
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
    PrototypeRegistry::addFcnPrototype(protoName, std::move(fcn.releasePrototype()));
    llvm::Function* function = PrototypeRegistry::getFunction(protoName, *this);

    if (!function) {
        return nullptr;
    }

    // Create a new basic block for the function body
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(*context, "entry", function);
    builder->SetInsertPoint(bb);

    namedValues.clear();
    for (auto &arg : function->args()) {
        // Map the argument names to their corresponding LLVM values
        namedValues[arg.getName().str()] = &arg;
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