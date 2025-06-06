#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"

#include "AST/PrototypeRegistry.hpp"
#include "AST/ValueVisitor.hpp"
#include "frontend/Parser.hpp"
#include "JIT/KaleidoscopeJITCopy.h"

using namespace lang;
using namespace llvm;
using namespace llvm::orc;

class Driver {
public:
    Driver(const std::string& moduleName, std::istream& stream, bool isInteractive, bool useJIT = true) 
        : ModuleName(moduleName), lexer(stream), parser(lexer), 
            interactive(isInteractive), isJIT(useJIT) {

        jit = ExitOnErr(KaleidoscopeJIT::Create());

        // Prime the first token
        logInteractive("ready> ");
        lexer.advance();   
    }

    void initializeModule() {
        context = std::make_unique<LLVMContext>();
        module = std::make_unique<Module>(ModuleName, *context);
        module->setDataLayout(jit->getDataLayout());

        // bad bad bad
        PrototypeRegistry::get()->setModule(module.get());

        builder = std::make_unique<IRBuilder<>>(*context);
        visitor = std::make_unique<CodegenVisitor>(context.get(), module.get(), builder.get());
    }


    void initilizeModuleAndManagers() {
        initializeModule();
        initializeManagers();
    }

    Module* getModule() const {
        return module.get();
    }

    /// top ::= definition | external | expression | ';'
    void MainLoop() {
        while (true) {
            switch (lexer.getCurrentToken()) {
                case tok_eof:
                    logInteractive("Goodbye!\n");
                    return;
                case tok_semicolon: // ignore top-level semicolons.
                    logInteractive("ready> ");
                    lexer.advance();
                    break;
                case tok_def:
                    HandleDefinition();
                    break;
                case tok_extern:
                    HandleExtern();
                    break;
                default:
                    HandleTopLevelExpression();
                    break;
            }
        }
    }
private:
    void initializeManagers() {
        // Create pass and analysis managers
        fpm = std::make_unique<FunctionPassManager>();
        lam = std::make_unique<LoopAnalysisManager>();
        fam = std::make_unique<FunctionAnalysisManager>();
        cgam = std::make_unique<CGSCCAnalysisManager>();
        mam = std::make_unique<ModuleAnalysisManager>();
        pic = std::make_unique<PassInstrumentationCallbacks>();
        si = std::make_unique<StandardInstrumentations>(*context, true);

        si->registerCallbacks(*pic, mam.get());
        
        // Add transform passes.
        // Promote allocas to registers
        fpm->addPass(PromotePass());
        // Do simple peephole optimizatons and bit-twiddling optimizations
        fpm->addPass(InstCombinePass());
        // Reassociate expressions
        fpm->addPass(ReassociatePass());
        // Eliminate common SubExpressions
        fpm->addPass(GVNPass());
        // Simplify the control flow graph (deleting unreachable blocks, etc.)
        fpm->addPass(SimplifyCFGPass());

        visitor->setFAM(fam.get());
        visitor->setFPM(fpm.get());

        // Register the analysis passes used in the transform passes
        PassBuilder pb;
        pb.registerModuleAnalyses(*mam);
        pb.registerFunctionAnalyses(*fam);
        pb.crossRegisterProxies(*lam, *fam, *cgam, *mam);
    }

    void HandleDefinition() {
        if (auto fcn = parser.parseDefinition()) {
            if (auto fcnIR = fcn->accept(*visitor)) {
                dumpIR(fcnIR, "Parsed a function definition.");
                if (isJIT) {
                    ExitOnErr(jit->addModule(
                        ThreadSafeModule(std::move(module), std::move(context))
                    ));
                    initilizeModuleAndManagers();
                }
            }
        } else {
            lexer.advance();
        }
    }

    void HandleExtern() {
        if (auto fcnProto = parser.parseExtern()) {
            if (auto fcnIR = fcnProto->accept(*visitor)) {
                dumpIR(fcnIR, "Parsed an extern");
                PrototypeRegistry::addFcnPrototype(fcnProto->getName(), std::move(fcnProto));
            }
        } else {
          // Skip token for error recovery.
          lexer.advance();
        }
    }

    void HandleTopLevelExpression() {
        // Evaluate a top-level expression into an anonymous function.
        if (auto fcnAST = parser.parseTopLevelExpr()) {
            if (auto fcnIR = fcnAST->accept(*visitor)) {
                dumpIR(fcnIR, "Parsed a top-level expr");

                // TODO: Actually separate out JIT code
                if (isJIT) {
                    // Create a ResourceTracker for JIT memory allocated to the
                    // anonymous expression so we can free it after execution
                    auto rt = jit->getMainJITDylib().createResourceTracker();
                    
                    auto tsm = ThreadSafeModule(std::move(module), 
                                                            std::move(context));
                    ExitOnErr(jit->addModule(std::move(tsm), rt));
                    
                    // Module has been added to JIT and can't be modified, open
                    // a new module for subsequent code
                    initilizeModuleAndManagers();

                    // Search JIT for anon exprs
                    auto exprSym = ExitOnErr(jit->lookup("main"));
                    assert(exprSym.getAddress() && "Function not found");
                    
                    // Get the symbol's address and cast it to the correct type
                    // to call it as a native function
                    double (*FP)() = exprSym.getAddress().toPtr<double (*)()>();
                    fprintf(stderr, "Evaluated to %f\n", FP());

                    // Remove the anonymous expression module from the JIT since
                    // we don't support re-evaluation of top level exprs
                    ExitOnErr(rt->remove());
                }

            }
        } else {
            // Skip token for error recovery.
            lexer.advance();
        }
    }

    void logInteractive(const char* format, ...) const {
        if (!interactive) return;

        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
    }

    void dumpIR(Value* IR, const char* parseMsg) {
        if (interactive) {
            fprintf(stderr, "%s\n", parseMsg);
            IR->print(errs());
            fprintf(stderr, "\n");
        }
    }

    Parser parser;
    Lexer lexer;
    ExitOnError ExitOnErr;
    std::string ModuleName;
    bool interactive;
    bool isJIT;

    std::unique_ptr<CodegenVisitor> visitor;
    std::unique_ptr<LLVMContext> context;
    std::unique_ptr<Module> module;
    std::unique_ptr<IRBuilder<>> builder;

    std::unique_ptr<FunctionPassManager> fpm;
    std::unique_ptr<LoopAnalysisManager> lam;
    std::unique_ptr<FunctionAnalysisManager> fam;
    std::unique_ptr<CGSCCAnalysisManager> cgam;
    std::unique_ptr<ModuleAnalysisManager> mam;
    std::unique_ptr<PassInstrumentationCallbacks> pic;
    std::unique_ptr<StandardInstrumentations> si;

    std::unique_ptr<KaleidoscopeJIT> jit;
    std::unordered_map<std::string, FcnPrototype*> functionProtos;
};