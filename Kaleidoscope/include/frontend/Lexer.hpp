#pragma once

#include <cassert>
#include <iostream>
#include <string>

#include "debug/SourceLocation.hpp"

// Return token [0-255] for unknown chars, otherwise a known token
namespace lang {
enum Token : int {
    tok_open_paren = '(',
    tok_close_paren = ')',
    tok_comma = ',',
    tok_semicolon = ';',

    tok_eof = -1,

    tok_def = -2,
    tok_extern = -3,

    tok_identifier = -4,
    tok_number = -5,

    tok_if = -6,
    tok_then = -7,
    tok_else = -8,

    tok_for = -9,
    tok_in = -10,

    tok_binary = -11,
    tok_unary = -12,

    tok_var = -13,
};

static bool isnum(char c) {
    return isdigit(c) || c == '.';
}

static bool iseol(char c) {
    return c == '\n' || c == '\r' || c == EOF;
}

class Lexer {
public:
    Lexer(std::istream& input = std::cin) : fInput(input) {}

    Token getCurrentToken() const {
        return fCurTok;
    }

    const SourceLocation& getCurrentLoc() const {
        return fCurLoc;
    }

    Token advance() {
        return fCurTok = getTok();
    }

    void consume(Token tok) {
        assert(fCurTok == tok && "Lexer::consume called with wrong token");
        advance(); // Advance to the next token
    }

    // Get the identifier string if tok_identifier
    const std::string &getIdentifierStr() const {
        return fIdentiferStr;
    }

    // Get the number value if tok_number
    double getNumVal() const {
        return fNumVal;
    }

private:
    Token fCurTok = tok_eof;
    std::string fIdentiferStr; // Filled in if tok_identifier
    std::istream& fInput; // Input stream, default is std::cin
    Token fLastChar = Token(' ');
    double fNumVal; // Filled in if tok_number

    SourceLocation fCurLoc;
    SourceLocation fLexLoc = {1, 0};

    int getNextChar() {
        return fInput.get();
    }

    Token next() {
        int lastChar = getNextChar();

        if (lastChar == '\n' || lastChar == '\r') {
            fLexLoc.Line++;
            fLexLoc.Col = 0;
        } else {
            fLexLoc.Col++;
        }
        return Token(lastChar);
    }

    Token getTok() {
        // Skip whitespace
        while (isspace(fLastChar)) {
            fLastChar = next();
        }

        fCurLoc = fLexLoc;

        if (std::isalpha(fLastChar)) {
            fIdentiferStr = (char)fLastChar;
            while (std::isalnum((fLastChar = next()))) {
                fIdentiferStr += (char)fLastChar;
            }
            return getTokFromWord(fIdentiferStr);
        }

        // Match [0-9.]+ , needs to be improved to handle more cases
        if (isnum(fLastChar)) {
            std::string NumStr;
            int decimal = 0;
            do {
                NumStr += fLastChar;
                fLastChar = next();
                if (fLastChar == '.') {
                    decimal++;
                    assert(decimal < 2 && "Cannot handle multiple decimals in a number");
                }
            } while (isnum(fLastChar));
            fNumVal = strtod(NumStr.c_str(), nullptr);
            return tok_number;
        }

        if (fLastChar == '#') {
            // Comment until end of line
            do {
                fLastChar = next();
            } while (!iseol(fLastChar));

            if (fLastChar != EOF) {
                return getTok(); // Recurse to get next token
            }
        }

        if (fLastChar == EOF) {
            return tok_eof; // End of file
        }

        Token ThisChar = fLastChar;
        fLastChar = next(); // Get next character
        return ThisChar;
    }

    Token getTokFromWord(const std::string &word) {
        if (word == "def") {
            return tok_def;
        }
        if (word == "extern") {
            return tok_extern;
        }
        if (word == "if") {
            return tok_if;
        }
        if (word == "then") {
            return tok_then;
        }
        if (word == "else") {
            return tok_else;
        }
        if (word == "for") {
            return tok_for;
        }
        if (word == "in") {
            return tok_in;
        }
        if (word == "binary") {
            return tok_binary;
        }
        if (word == "unary") {
            return tok_unary;
        }
        if (word == "var") {
            return tok_var;
        }
        return tok_identifier;
    }
};

} // namespace lang

