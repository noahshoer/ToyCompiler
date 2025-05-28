#pragma once

#include "AST/Expr.hpp"
#include "AST/Fcn.hpp"
#include "Lexer.hpp"

namespace lang {
static std::unique_ptr<Expr> LogError(const char* str) {
    fprintf(stderr, "LogError: %s\n", str);
    return nullptr;
}

static std::unique_ptr<Expr> LogError(const std::string& str) {
    return LogError(str.c_str());
}

static std::unique_ptr<FcnPrototype> LogErrorP(const char* str) {
    LogError(str);
    return nullptr;
}

static std::unique_ptr<FcnPrototype> LogErrorP(const std::string& str) {
    return LogErrorP(str.c_str());
}

class Parser {
public:
    Parser(Lexer& lexer) : fLexer(lexer) {}

private:
    Lexer fLexer;

    inline Token current() { return fLexer.getCurrentToken(); }
    inline void next()  { fLexer.getNextToken(); }

    std::unique_ptr<Expr> parseExpression() {

    }

    std::unique_ptr<Expr> parseIdentifierExpr() {
        std::string idName = fLexer.getIdentifierStr();
        next() ; // Consume the identifier token

        if (current() != tok_open_paren) {
            return std::make_unique<VariableExpr>(idName);
        }

        // Is a CallExpr
        next() ; // Consume '('
        std::vector<std::unique_ptr<Expr>> args;
        if (current() != tok_close_paren) {
            while (true) {
                if (auto arg = parseExpression()) {
                    args.push_back(std::move(arg));
                } else {
                    return nullptr; // Error in argument parsing
                }

                if (current() == tok_close_paren) {
                    break; // end of arguments
                }

                if (current() != tok_comma) {
                    return LogError("Expected ')' or ',' in argument list");
                }
                next(); // Consume ','
            }
        }

        next(); // Consume ')'
        return std::make_unique<CallExpr>(idName, std::move(args));
    }

    std::unique_ptr<Expr> parseNumberExpr() {
        auto result = std::make_unique<NumberExpr>(fLexer.getNumVal());
        next(); // Consume the number token
        return std::move(result);
    }

    std::unique_ptr<Expr> parseParenExpr() {
        next(); // Consume '('
        auto expr = parseExpression();
        if (!expr) return nullptr;

        if (current() != tok_close_paren) {
            return LogError("Expected ')'");
        }
        next(); // Consume ')'
        return expr;
    }

    std::unique_ptr<Expr> parsePrimary() {
        switch (current()) {
            default:
                return LogError("Unknown token when expecting an expression");
            case tok_identifier:
                return parseIdentifierExpr();
            case tok_number:
                return parseNumberExpr();
            case tok_open_paren:
                return parseParenExpr();
        }
    }

    // Used for Operator-Precedence Parsing, as binary operators
    // have an expected "precedence" in mathematics that we want to respect.
    int getTokenPrecedence() {
        if (!isascii(current())) {
            return -1;
        }

        switch (static_cast<char>(current())) {
            case '<':
                return 10;
            case '+':
            case '-':
                return 20;
            case '*':
                return 40;
            default:
                return -1; // Not a binary operator
        }
    }
};
} // namespace lang
