#include "frontend/Parser.hpp"

using namespace lang;

static void HandleDefinition(Lexer &lexer, Parser &parser) {
  if (parser.parseDefinition()) {
    fprintf(stdout, "Parsed a function definition.\n");
  } else {
    // Skip token for error recovery.
    lexer.advance();
  }
}

static void HandleExtern(Lexer &lexer, Parser &parser) {
  if (parser.parseExtern()) {
    fprintf(stdout, "Parsed an extern\n");
  } else {
    // Skip token for error recovery.
    lexer.advance();
  }
}

static void HandleTopLevelExpression(Lexer &lexer, Parser &parser) {
  // Evaluate a top-level expression into an anonymous function.
  if (parser.parseTopLevelExpr()) {
    fprintf(stdout, "Parsed a top-level expr\n");
    fflush(stdout);
  } else {
    // Skip token for error recovery.
    lexer.advance();
  }
}

/// top ::= definition | external | expression | ';'
static void MainLoop(Lexer &lexer, Parser &parser) {
  while (true) {
    switch (lexer.getCurrentToken()) {
      case tok_eof:
        fprintf(stdout, "Goodbye!\n");
        return;
      case tok_semicolon: // ignore top-level semicolons.
        fprintf(stdout, "ready> ");
        lexer.advance();
        break;
      case tok_def:
        HandleDefinition(lexer, parser);
        break;
      case tok_extern:
        HandleExtern(lexer, parser);
        break;
      default:
        HandleTopLevelExpression(lexer, parser);
        break;
    }
  }
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main() {
  Lexer lexer;
  Parser parser(lexer);
  // Prime the first token.
  fprintf(stdout, "ready> ");
  lexer.advance();

  // Run the main "interpreter loop" now.
  MainLoop(lexer, parser);

  return 0;
}