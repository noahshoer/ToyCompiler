#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Expr.hpp"

class FcnPrototype {
    std::string name;
    std::vector<std::string> args;

public:
    FcnPrototype(const std::string &Name, std::vector<std::string> Args)
        : name(Name), args(std::move(Args)) {}
        
    const std::string &getName() const {
        return name;
    }
};

class Fcn {
    std::unique_ptr<FcnPrototype> prototype;
    std::unique_ptr<Expr> body;

public:
    Fcn(std::unique_ptr<FcnPrototype> Prototype, std::unique_ptr<Expr> Body)
        : prototype(std::move(Prototype)), body(std::move(Body)) {}
    
};