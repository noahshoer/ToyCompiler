
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
    llvm::AllocaInst* allocaInst = namedValues[expr.getName()];
    if (!allocaInst) {
        logError("Variable '" + expr.getName() + "' is unknown");
        return nullptr;
    }
    return builder->CreateLoad(allocaInst->getAllocatedType(), allocaInst,
                                expr.getName());
}

llvm::Value* CodegenVisitor::visitBinaryExpr(BinaryExpr &expr) {
    // Assignments are a special case since the LHS ins't an expression
    if (expr.getOp() == '=') {
        VariableExpr* lhse = static_cast<VariableExpr*>(expr.getLHS());
        if (!lhse) {
            logError("Destination of '=' must be a variable");
            return nullptr;
        }
        auto val = expr.getRHS()->accept(*this);
        if (!val) {
            return nullptr;
        }

        auto var = namedValues[lhse->getName()];
        if (!var) {
            logError("Unkown variable name");
            return nullptr;
        }
        
        builder->CreateStore(val, var);
        return val;
    }
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
    // Insert loop header block after the current block
    llvm::Function* function = builder->GetInsertBlock()->getParent();

    // Create an alloca for the variable in the entry block
    llvm::AllocaInst* allocaInst = createEntryBlockAlloca(function,
                                    expr.getVarName());

    // Emit the start code, variable is not in scope
    llvm::Value* startVal = expr.getStart()->accept(*this);
    if (!startVal) {
        return nullptr;
    }

    // Store the value into the alloca
    builder->CreateStore(startVal, allocaInst);

    // Make the new basic block for the loop header, inserting after current block
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(*context, "loop", 
                                                    function);

    // Explicitly add a fall through from the current block to the loop
    builder->CreateBr(loopBB);

    // No PHI Node since using built-in LLVM SSA form. Within the loop, the variable is
    // defined equal to the PHI node. Shadow the var if it exists
    llvm::AllocaInst* oldAlloca = namedValues[expr.getVarName()];
    setNamedValue(expr.getVarName(), allocaInst);

    // Emit the body, ignoring the computed value but not allowing an error
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

    // Compute the end condition
    llvm::Value* endCond = expr.getEnd()->accept(*this);
    if (!endCond) {
        return nullptr;
    }

    // Reload, increment, and restore the alloca. This handles the case where the body
    // of the loop mutates the variable.
    llvm::Value* curVar = builder->CreateLoad(allocaInst->getAllocatedType(),
                                allocaInst, expr.getVarName());
    llvm::Value* nextVar = builder->CreateFAdd(curVar, stepVal, "nextvar");
    builder->CreateStore(nextVar, allocaInst);
    
    // Convert condition to a bool by comparing not equal to 0.0
    endCond = builder->CreateFCmpONE(endCond, llvm::ConstantFP::get(*context, 
                                    llvm::APFloat(0.0)), "loopcond");

    // Create the after loop blcok and insert it
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(*context, "afterloop", 
                                                    function);
    
    // Insert conditional branch into the end of loopend BB
    builder->CreateCondBr(endCond, loopBB, afterBB);

    // Any new code inserted in after BB
    builder->SetInsertPoint(afterBB);

    // Restore unshadowed variable
    if (oldAlloca) {
        setNamedValue(expr.getVarName(), oldAlloca);
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
        // Create an alloca for the arg
        llvm::AllocaInst* argAllocaInst = createEntryBlockAlloca(function, arg.getName());

        // Store value in the alloca
        builder->CreateStore(&arg, argAllocaInst);

        // Map the argument names to their corresponding LLVM values
        setNamedValue(arg.getName().str(), argAllocaInst);
    }

    if (auto retVal = fcn.getBody()->accept(*this)) {
        // If the function body returns a value, create a return instruction
        builder->CreateRet(retVal);

        // Validates the generated code
        llvm::verifyFunction(*function);

        // Optimize the function
        if (fpm) {
            fpm->run(*function, *fam);
        }
        return function;
    }

    function->eraseFromParent(); // If the body is invalid, remove the function
    return function;
}

llvm::Value* CodegenVisitor::visitVarExpr(VarExpr &expr) {
    std::vector<llvm::AllocaInst*> oldBindings;

    llvm::Function* function = builder->GetInsertBlock()->getParent();

    // Register all vars and emit their initializer
    for (const auto& var : expr.getVarNames()) {
        const std::string& varName = var.first;
        Expr* init = var.second;

        // Emit the initalizer before adding the variable to scope to
        // prevent the initializer from referencing the variable itself,
        // and permit stuff like:
        // var a = 1 in 
        //  var a = a in ... # refers to outer 'a'
        llvm::Value* initVal;
        if (init) {
            initVal = init->accept(*this);
            if (!initVal) {
                return nullptr;
            }
        } else {
            // Default initialize to 0.0
            initVal = llvm::ConstantFP::get(*context, 
                                            llvm::APFloat(0.0));
        }

        llvm::AllocaInst* allocaInst = createEntryBlockAlloca(function, varName);

        oldBindings.push_back(namedValues[varName]);
        setNamedValue(varName, allocaInst);
    }

    llvm::Value* bodyVal = expr.getBody()->accept(*this);
    for (int i = 0; i < expr.getVarNames().size(); ++i) {
        setNamedValue(expr.getVarNames()[i].first, oldBindings[i]);
    }

    return bodyVal;
}