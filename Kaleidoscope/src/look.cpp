#include <fstream>  

#include "llvm/Support/TargetSelect.h"

#include "frontend/Driver.hpp"

//===----------------------------------------------------------------------===//
// Main driver code for JIT execution
//===----------------------------------------------------------------------===//

int main(int argc, char* argv[]) {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
    
    bool compileFile = argc >= 2;
    std::unique_ptr<std::ifstream> fileStream;
    std::istream* stream = &std::cin;
    std::filesystem::path filepath;
    bool interactive = true;
    if (compileFile) {
        const char* filename = argv[1];
        std::cout << "You passed in: " << filename << "\n";

        fileStream = std::make_unique<std::ifstream>(filename);
        filepath = filename;
        if (!fileStream) {
            std::cerr << "Error opening file: " << filename << "\n";
            return 1;
        }
        stream = fileStream.get();
        interactive = false;
    }

    Driver driver("cool stuff", *stream, interactive);
    driver.initilizeModuleAndManagers();
    driver.MainLoop();

    return 0;
}
