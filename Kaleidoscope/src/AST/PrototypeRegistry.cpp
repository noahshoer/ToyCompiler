#include "AST/Fcn.hpp"
#include "AST/PrototypeRegistry.hpp"
#include "AST/ValueVisitor.hpp"

std::unique_ptr<PrototypeRegistry> PrototypeRegistry::instance = nullptr;

void PrototypeRegistry::addFcnPrototype(const std::string& name, std::unique_ptr<FcnPrototype> fcnProto) {
    get()->fcnPrototypes[name] = std::move(fcnProto);
}

llvm::Function* PrototypeRegistry::getFunction(const std::string& name, CodegenVisitor& visitor) {
    if (auto *fcn = get()->module->getFunction(name)) {
        return fcn;
    }

    if (auto fcnProto = get()->getFcnPrototype(name)) {
        return static_cast<llvm::Function*>(fcnProto->accept(visitor));
    }
    return nullptr;
}

FcnPrototype* PrototypeRegistry::getFcnPrototype(const std::string& name) {
    auto fcnProtoIt = fcnPrototypes.find(name);
    if (fcnProtoIt != fcnPrototypes.end()) {
        return fcnProtoIt->second.get();
    }
    return nullptr;
}