#pragma once

#include "AST/Expr.hpp"
#include "AST/Fcn.hpp"
#include "AST/Precedence.hpp"
#include "Lexer.hpp"

namespace lang {

class Parser {
public:
    Parser(Lexer& lexer) : fLexer(lexer) {}

    /// Parses a function definition, which is of the form:
    ///     def <prototype> <expression>
    std::unique_ptr<Fcn> parseDefinition() {
        if (fLexer.getCurrentToken() != tok_def) {
            return logErrorAndReturnNull<Fcn>("Expected 'def' keyword for function definition");
        }
        fLexer.consume(tok_def);
        auto proto = parsePrototype();
        if (!proto) {
            return nullptr; // Error in prototype parsing
        }

        if (auto expr = parseExpression()) {
            return std::make_unique<Fcn>(std::move(proto), std::move(expr));
        } else {
            return logErrorAndReturnNull<Fcn>("Expected expression in function definition");
        }   
    }

    /// Parses an extern function prototype, which is of the form:
    ///     extern <prototype>
    std::unique_ptr<FcnPrototype> parseExtern() {
        if (fLexer.getCurrentToken() != tok_extern) {
            return logErrorAndReturnNull<FcnPrototype>("Expected 'extern' keyword for function prototype");
        }
        fLexer.consume(tok_extern);
        return parsePrototype();
    }

    /// Parses a top-level expression, which is of the form:
    ///     <expression>
    std::unique_ptr<Fcn> parseTopLevelExpr() {
        if (auto expr = parseExpression()) {
            auto proto = std::make_unique<FcnPrototype>("__anon_expr", std::vector<std::string>());
            return std::make_unique<Fcn>(std::move(proto), std::move(expr));
        }
        return nullptr;
    }

private:
    Lexer& fLexer;

    template<typename R>
    inline std::unique_ptr<R> logErrorAndReturnNull(const char* str) {
        fprintf(stderr, "ParseError: %s\n", str);
        return nullptr;
    }
    
    /// An IdentifierExpr is of the form:
    ///     <identifier> for a VariableExpr, or
    ///     <identifier> ( <expression> , ... ) for a CallExpr.
    std::unique_ptr<Expr> parseIdentifierExpr() {
        if (fLexer.getCurrentToken() != tok_identifier) {
            return logErrorAndReturnNull<Expr>("Expected identifier");
        }

        std::string idName = fLexer.getIdentifierStr();
        if (fLexer.advance() != tok_open_paren) {
            return std::make_unique<VariableExpr>(idName);
        }

        std::vector<std::unique_ptr<Expr>> args;
        if (!gatherCallExprArgs(args)) {
            return nullptr;
        }

        if (fLexer.getCurrentToken() != tok_close_paren) {
            return nullptr;
        }

        fLexer.advance();
        return std::make_unique<CallExpr>(idName, std::move(args));
    }

    bool gatherCallExprArgs(std::vector<std::unique_ptr<Expr>>& args) {
        if (fLexer.advance() == tok_close_paren) {
            return true;
        }

        while (true) {
            if (auto arg = parseExpression()) {
                args.push_back(std::move(arg));
            } else {
                return false; // Error in argument parsing
            }

            if (fLexer.getCurrentToken() == tok_close_paren) {
                return true; // end of arguments
            }

            if (fLexer.getCurrentToken() != tok_comma) {
                logErrorAndReturnNull<Expr>("Expected ')' or ',' in argument list");
                return false;
            }
            fLexer.consume(tok_comma);
        }
    }

    /// A NumberExpr is of the form:
    ///     <number>
    std::unique_ptr<Expr> parseNumberExpr() {
        auto result = std::make_unique<NumberExpr>(fLexer.getNumVal());
        fLexer.consume(tok_number);
        return std::move(result);
    }

    /// A ParenExpr is of the form:
    ///     (<expression>)
    std::unique_ptr<Expr> parseParenExpr() {
        fLexer.consume(tok_open_paren);
        auto expr = parseExpression();
        if (!expr) 
            return nullptr;

        if (fLexer.getCurrentToken() != tok_close_paren) {
            return logErrorAndReturnNull<Expr>("Expected ')'");
        }
        fLexer.consume(tok_close_paren);
        return expr;
    }

    /// An Expression is a sequence of primary expressions
    /// combined with binary operators, respecting operator precedence.
    /// It is of the form:
    ///     <primary> <binOp> <primary> <binOp> <primary> ...
    std::unique_ptr<Expr> parseExpression() {
        auto LHS = parseUnary();
        if (!LHS) 
            return nullptr;

        return parseBinOpRHS(0, std::move(LHS));
    }

    /// A PrimaryExpr is either:
    ///     - An IdentifierExpr (VariableExpr or CallExpr)
    ///     - A NumberExpr
    ///     - A ParenExpr
    std::unique_ptr<Expr> parsePrimary() {
        switch (fLexer.getCurrentToken()) {
            default:
                return logErrorAndReturnNull<Expr>("Unknown token when expecting an expression");
            case tok_identifier:
                return parseIdentifierExpr();
            case tok_number:
                return parseNumberExpr();
            case tok_open_paren:
                return parseParenExpr();
            case tok_if:
                return parseIfExpr();
            case tok_for:
                return parseForExpr();
            case tok_var:
                return parseVarExpr();
        }
    }

    // Used for Operator-Precedence Parsing, as binary operators
    // have an expected "precedence" in mathematics that we want to respect.
    int getTokenPrecedence() {
        if (!isascii(fLexer.getCurrentToken())) {
            return -1;
        }

        auto it = BIN_OP_PRECEDENCE.find(static_cast<char>(fLexer.getCurrentToken()));
        if (it == BIN_OP_PRECEDENCE.end()) {
            return -1;
        }
        return it->second;
    }

    /// Recursively parse binary operators with precedence.
    /// This function assumes that LHS is already parsed and
    /// that the current token is a binary operator with precedence >= exprPrec.
    ///
    /// It will parse the right-hand side (RHS) of the binary operation,
    /// and continue parsing if the next token is also a binary operator
    /// with higher or equal precedence.
    ///
    /// BinaryExprs are of the form:
    ///     <binOp> <primary>*
    std::unique_ptr<Expr> parseBinOpRHS(int exprPrec, std::unique_ptr<Expr> LHS) {
        while (true) {
            int tokPrec = getTokenPrecedence();
            if (tokPrec < exprPrec) {
                return LHS; // No more binary operators with higher precedence
            }

            int binOp = fLexer.getCurrentToken();
            fLexer.consume(Token(binOp)); // Consume the operator token

            auto RHS = parseUnary();
            if (!RHS) 
                return nullptr;

            int nextPrec = getTokenPrecedence();
            if (tokPrec < nextPrec) {
                RHS = parseBinOpRHS(tokPrec + 1, std::move(RHS));
                if (!RHS) 
                    return nullptr;
            }

            LHS = std::make_unique<BinaryExpr>(binOp, std::move(LHS), std::move(RHS));
        }
    }

    /// Parses a function prototype, which is of the form:
    ///     <identifier> ( <identifier> , ... )
    ///     <binary><CHAR> number? (id id)
    std::unique_ptr<FcnPrototype> parsePrototype() {
        std::string fcnName;

        unsigned Kind = 0; // 0 = ID, 1 = Unary, 2 = Binary
        unsigned BinaryPrecedence = 30;

        switch (fLexer.getCurrentToken()) {
            default:
                return logErrorAndReturnNull<FcnPrototype>("Expected function name in prototype");
            case tok_identifier:
                fcnName = fLexer.getIdentifierStr();
                Kind = 0;
                fLexer.consume(tok_identifier);
                break;
            case tok_unary:
                fLexer.consume(tok_unary);
                if (!isascii(fLexer.getCurrentToken()) || std::isalnum(fLexer.getCurrentToken())) {
                    return logErrorAndReturnNull<FcnPrototype>("Expected unary operator");
                }
                fcnName = "unary";
                fcnName += (char)fLexer.getCurrentToken();
                Kind = 1;
                fLexer.advance();
                break;
            case tok_binary:
                fLexer.consume(tok_binary);
                if (!isascii(fLexer.getCurrentToken()) || std::isalnum(fLexer.getCurrentToken())) {
                    return logErrorAndReturnNull<FcnPrototype>("Expected binary operator");
                }
                fcnName = "binary";
                fcnName += (char)fLexer.getCurrentToken();
                Kind = 2;
                fLexer.advance();

                if (fLexer.getCurrentToken() == tok_number) {
                    if (fLexer.getNumVal() < 1 || fLexer.getNumVal() > 100) {
                        return logErrorAndReturnNull<FcnPrototype>("Invalid precedence, must be between 1-100");
                    }
                    BinaryPrecedence = (unsigned)fLexer.getNumVal();
                    fLexer.consume(tok_number);
                }
                break;
        }
        if (fLexer.getCurrentToken() != tok_open_paren) {
            return logErrorAndReturnNull<FcnPrototype>("Expected '(' in prototype");
        }

        // What about commas?
        std::vector<std::string> argNames;
        while (fLexer.advance() == tok_identifier) {
            argNames.push_back(fLexer.getIdentifierStr());
        }

        if (fLexer.getCurrentToken() != tok_close_paren) {
            return logErrorAndReturnNull<FcnPrototype>("Expected ')' in function prototype");
        }
        fLexer.consume(tok_close_paren);

        if (Kind && argNames.size() != Kind) {
            return logErrorAndReturnNull<FcnPrototype>("Invalid number of operands for operator");
        }
        return std::make_unique<FcnPrototype>(fcnName, std::move(argNames), 
                                    Kind != 0, BinaryPrecedence);
    }

    std::unique_ptr<Expr> parseIfExpr() {
        fLexer.consume(tok_if);

        auto Cond = parseExpression();
        if (!Cond) {
            return nullptr;
        }

        if (fLexer.getCurrentToken() != tok_then) {
            return logErrorAndReturnNull<IfExpr>("Expected a then");
        }
        fLexer.advance();

        auto Then = parseExpression();
        if (!Then) {
            return nullptr;
        }

        if (fLexer.getCurrentToken() != tok_else) {
            return logErrorAndReturnNull<IfExpr>("Expected else");
        }
        fLexer.advance();

        auto Else = parseExpression();
        if (!Else) {
            return nullptr;
        }

        return std::make_unique<IfExpr>(std::move(Cond), std::move(Then), std::move(Else));
    }

    std::unique_ptr<Expr> parseForExpr() {
        fLexer.consume(tok_for);

        if (fLexer.getCurrentToken() != tok_identifier) {
            return logErrorAndReturnNull<ForExpr>("Expected identifier after for");
        }

        std::string idName = fLexer.getIdentifierStr();
        fLexer.advance();

        if (fLexer.getCurrentToken() != '=') {
            return logErrorAndReturnNull<ForExpr>("Expected '=' after for identifer");
        }
        fLexer.advance();

        // Don't use parseExpression to ensure parseUnary is not called
        // upfront, that can swallow a missing comma
        auto start = parsePrimary();
        if (!start) {
            return nullptr;
        }
        if (fLexer.getCurrentToken() != tok_comma) {
            return logErrorAndReturnNull<ForExpr>("Expected ',' after for start value");
        }
        fLexer.advance();

        auto end = parseExpression();
        if (!end) {
            return nullptr;
        }

        std::unique_ptr<Expr> step;
        if (fLexer.getCurrentToken() == tok_comma) {
            fLexer.advance();
            step = parseExpression();
            if (!step) {
                return nullptr;
            }
        }

        if (fLexer.getCurrentToken() != tok_in) {
            return logErrorAndReturnNull<ForExpr>("Expected 'in' after for");
        }
        fLexer.advance();

        auto body = parseExpression();
        if (!body) {
            return nullptr;
        }
        
        return std::make_unique<ForExpr>(idName, std::move(start), 
                        std::move(end),std::move(step), std::move(body));
    }

    /// unary of the form:
    ///     <primary>
    ///     unary<op>
    std::unique_ptr<Expr> parseUnary() {
        auto curTok = fLexer.getCurrentToken();
        if (!isascii(curTok) || curTok == tok_open_paren || curTok == tok_comma) {
            return parsePrimary();
        }

        fLexer.advance();
        if (auto operand = parseUnary()) {
            return std::make_unique<UnaryExpr>(curTok, std::move(operand));
        }
        return nullptr;
    }

    std::unique_ptr<Expr> parseVarExpr() {
        fLexer.consume(tok_var);

        VarNameVector varNames;

        if (fLexer.getCurrentToken() != tok_identifier) {
            return logErrorAndReturnNull<VarExpr>("expected identifier after var");
        }

        while (true) {
            std::string name = fLexer.getIdentifierStr();
            fLexer.consume(tok_identifier);

            ExprUPtr init;
            if (fLexer.getCurrentToken() == '=') {
                fLexer.advance();

                init = parseExpression();
                if (!init) return nullptr;
            }

            varNames.push_back(std::make_pair(name, std::move(init)));

            // End of var list, exist the loop
            if (fLexer.getCurrentToken() != tok_comma) {
                break;
            }
            fLexer.consume(tok_comma);

            if (fLexer.getCurrentToken() != tok_identifier) {
                return logErrorAndReturnNull<VarExpr>("epected identifer list after var");
            }
        }

        if (fLexer.getCurrentToken() != tok_in) {
            return logErrorAndReturnNull<VarExpr>("expected 'in' keyword after 'var");
        }
        fLexer.consume(tok_in);

        auto body = parseExpression();
        if (!body) {
            return nullptr;
        }

        return std::make_unique<VarExpr>(std::move(varNames), std::move(body));
    }
};
} // namespace lang
