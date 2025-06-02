#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

class FcnPrototype;
class CodegenVisitor;

class PrototypeRegistry {
public:
    static PrototypeRegistry* get() {
        if (!instance) {
            instance.reset(new PrototypeRegistry());
        }
        return instance.get();
    }

    static void reset() {
        instance.reset();
    }

    void setModule(llvm::Module* aModule) {
        get()->module = aModule;
    }

    static void addFcnPrototype(const std::string& name, std::unique_ptr<FcnPrototype> fcnProto);
    static llvm::Function* getFunction(const std::string& name, CodegenVisitor& visitor);

    PrototypeRegistry(const PrototypeRegistry&) = delete;
    PrototypeRegistry& operator=(const PrototypeRegistry&) = delete;
    virtual ~PrototypeRegistry() = default;
    PrototypeRegistry(PrototypeRegistry&&) = delete;

private:
    PrototypeRegistry() = default;

    FcnPrototype* getFcnPrototype(const std::string& name);

    llvm::Module* module;
    std::unordered_map<std::string, std::unique_ptr<FcnPrototype>> fcnPrototypes;
    static std::unique_ptr<PrototypeRegistry> instance;
};
