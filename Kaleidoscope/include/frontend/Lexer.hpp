#pragma once

#include <cassert>
#include <iostream>
#include <string>

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
};

static bool isposnum(char c) {
    return isdigit(c) || c == '.';
}

static bool isnum(char c) {
    return isposnum(c) || c == '-';
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

    int getNextChar() {
        return fInput.get();
    }
    Token getTok() {
        // Skip whitespace
        while (isspace(fLastChar)) {
            fLastChar = Token(getNextChar());
        }

        if (std::isalpha(fLastChar)) {
            fIdentiferStr = (char)fLastChar;
            while (std::isalnum((fLastChar = Token(getNextChar())))) {
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
                fLastChar = Token(getNextChar());
                if (fLastChar == '.') {
                    decimal++;
                    assert(decimal < 2 && "Cannot handle multiple decimals in a number");
                }
            } while (isposnum(fLastChar));
            fNumVal = strtod(NumStr.c_str(), nullptr);
            return tok_number;
        }

        if (fLastChar == '#') {
            // Comment until end of line
            do {
                fLastChar = Token(getNextChar());
            } while (!iseol(fLastChar));

            if (fLastChar != EOF) {
                return getTok(); // Recurse to get next token
            }
        }

        if (fLastChar == EOF) {
            return tok_eof; // End of file
        }

        Token ThisChar = fLastChar;
        fLastChar = Token(getNextChar()); // Get next character
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
        return tok_identifier;
    }
};

} // namespace lang

