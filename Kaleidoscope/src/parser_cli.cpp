#include "AST/ASTVisitor.hpp"
#include "AST/ValueVisitor.hpp"
#include "frontend/Parser.hpp"

using namespace lang;

class Driver {
public:
    Driver(const std::string& moduleName) 
        : lexer(), parser(lexer) {
        
        makeModule(moduleName);
        visitor = std::make_unique<CodegenVisitor>(context.get(), module.get(), builder.get());
        // Prime the first token
        fprintf(stdout, "ready> ");
        lexer.advance();
    }

    llvm::Module* getModule() const {
        return module.get();
    }

    /// top ::= definition | external | expression | ';'
    void MainLoop() {
        while (true) {
            switch (lexer.getCurrentToken()) {
                case tok_eof:
                    fprintf(stdout, "Goodbye!\n");
                    return;
                case tok_semicolon: // ignore top-level semicolons.
                    fprintf(stdout, "ready> ");
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
    void makeModule(const std::string& moduleName) {
        context = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>(moduleName, *context);
        builder = std::make_unique<llvm::IRBuilder<>>(*context);
    }

    void HandleDefinition() {
        if (auto fcn = parser.parseDefinition()) {
            dump(fcn.get(), "Parsed a function definition.");
        }
    }

    void HandleExtern() {
        if (auto fcn = parser.parseExtern()) {
            dump(fcn.get(), "Parsed an extern");
        } else {
          // Skip token for error recovery.
          lexer.advance();
        }
    }

    void HandleTopLevelExpression() {
        // Evaluate a top-level expression into an anonymous function.
        if (auto fcn = parser.parseTopLevelExpr()) {
            dump(fcn.get(), "Parsed a top-level expr");

        } else {
            // Skip token for error recovery.
            lexer.advance();
        }
    }

    void dump(Fcn* node, const char* parseMsg) {
        if (auto IR = node->accept(*visitor)) {
            dumpImpl(IR, parseMsg);
        }
    }

    void dumpErase(Fcn* node, const char* parseMsg) {
        if (auto IR = node->accept(*visitor)) {
            dumpImpl(IR, parseMsg);
            static_cast<llvm::Function*>(IR)->eraseFromParent();
        }
    }

    void dump(FcnPrototype* node, const char* parseMsg) {
        if (auto IR = node->accept(*visitor)) {
            dumpImpl(IR, parseMsg);
        }
    }

    void dumpImpl(llvm::Value* IR, const char* parseMsg) {
        fprintf(stdout, parseMsg, "\n");
        IR->print(llvm::outs());
        fprintf(stdout, "\n");
    }

    Parser parser;
    Lexer lexer;
    std::unique_ptr<CodegenVisitor> visitor;
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
};



//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main() {
    Driver driver("cool stuff");
    
    driver.MainLoop();

    // Print out all of the generated code.
    driver.getModule()->print(llvm::outs(), nullptr);
    return 0;
}