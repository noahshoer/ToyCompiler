
#include "llvm/Support/TargetSelect.h"

#include "debug/DebugInfo.hpp"
#include "frontend/Driver.hpp"

#include <fstream>


//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    const char* filename = argv[1];
    std::cout << "You passed in: " << filename << "\n";

    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    std::ifstream inputFile(filename);
    if (!inputFile) {
        std::cerr << "Error opening file: " << filename << "\n";
        return 1;
    }

    Driver driver("cool stuff", inputFile, false, false);
    driver.initializeModule();
    DBuilder = std::make_unique<DIBuilder>(*driver.getModule());
    KSDbgInfo.TheCU = DBuilder->createCompileUnit(dwarf::DW_LANG_C,
        DBuilder->createFile(filename, "."), 
        "reflect", false, "", 0);
    driver.MainLoop();

    // Print out all of the generated code.
    std::error_code EC;
    llvm::raw_fd_ostream file("output.ll", EC, llvm::sys::fs::OF_None);

    DBuilder->finalize();
    if (EC) {
        llvm::errs() << "Error opening file: " << EC.message() << "\n";
    } else {
        driver.getModule()->print(file, nullptr);
    }
    return 0;
}