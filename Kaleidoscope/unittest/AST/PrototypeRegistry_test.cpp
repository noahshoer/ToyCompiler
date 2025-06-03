#include "gtest/gtest.h"

#include "AST/PrototypeRegistry.hpp"
#include "AST/Fcn.hpp"
#include "AST/ValueVisitor.hpp"

// Mock FcnPrototype
class MockFcnPrototype : public FcnPrototype {
public:
    MockFcnPrototype(const std::string& name, const std::vector<std::string>& args)
        : FcnPrototype(name, args), mockedFunction(nullptr) {}

    llvm::Value* accept(CodegenVisitor& visitor) {
        return mockedFunction; // Return pre-injected function pointer
    }

    void setMockedFunction(llvm::Function* func) {
        mockedFunction = func;
    }

private:
    llvm::Function* mockedFunction;
};

class PrototypeRegistryTest : public ::testing::Test {
protected:
    void SetUp() override {
        PrototypeRegistry::reset();
        context = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>("test", *context);
        PrototypeRegistry::get()->setModule(module.get());
    }

    void TearDown() override {
        PrototypeRegistry::reset();
    }

    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
};

TEST_F(PrototypeRegistryTest, GetFunctionReturnsExistingModuleFunction) {
    llvm::FunctionType* fType = llvm::FunctionType::get(llvm::Type::getDoubleTy(*context), false);
    llvm::Function* function = llvm::Function::Create(fType, llvm::Function::ExternalLinkage, "existingFunc", module.get());

    CodegenVisitor visitor(context.get(), module.get(), nullptr);
    llvm::Function* result = PrototypeRegistry::getFunction("existingFunc", visitor);
    EXPECT_EQ(result, function);
}

TEST_F(PrototypeRegistryTest, GetFunctionReturnsFunctionFromPrototype) {
    auto mockProto = std::make_unique<MockFcnPrototype>("mockFunc", std::vector<std::string>{});
    llvm::FunctionType* fType = llvm::FunctionType::get(llvm::Type::getDoubleTy(*context), false);
    llvm::Function* mockedFunction = llvm::Function::Create(fType, llvm::Function::ExternalLinkage, "mockFunc", module.get());

    mockProto->setMockedFunction(mockedFunction);
    PrototypeRegistry::addFcnPrototype("mockFunc", std::move(mockProto));

    CodegenVisitor visitor(context.get(), module.get(), nullptr);
    llvm::Function* result = PrototypeRegistry::getFunction("mockFunc", visitor);
    EXPECT_EQ(result, mockedFunction);
}

TEST_F(PrototypeRegistryTest, GetFunctionReturnsNullOnUnknownName) {
    CodegenVisitor visitor(context.get(), module.get(), nullptr);
    llvm::Function* result = PrototypeRegistry::getFunction("unknownFunc", visitor);
    EXPECT_EQ(result, nullptr);
}
