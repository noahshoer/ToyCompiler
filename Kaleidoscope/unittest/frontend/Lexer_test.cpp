#include "gtest/gtest.h"
#include "frontend/Lexer.hpp"
#include <sstream>

using namespace lang;

TEST(LexerTest, RecognizesEOF) {
    std::istringstream iss("");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_eof);
}

TEST(LexerTest, RecognizesDefKeyword) {
    std::istringstream iss("def");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_def);
}

TEST(LexerTest, RecognizesExternKeyword) {
    std::istringstream iss("extern");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_extern);
}

TEST(LexerTest, RecognizesIdentifier) {
    std::istringstream iss("foo");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_identifier);
    EXPECT_EQ(lexer.getIdentifierStr(), "foo");
}

TEST(LexerTest, RecognizesNumber) {
    std::istringstream iss("123");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_number);
    EXPECT_DOUBLE_EQ(lexer.getNumVal(), 123);
}

TEST(LexerTest, RecognizesNumberNegative) {
    std::istringstream iss("-123");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_number);
    EXPECT_DOUBLE_EQ(lexer.getNumVal(), -123);
}

TEST(LexerTest, SplitsOnExtraNegative)  {
    std::istringstream iss("-123-4");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_number);
    EXPECT_DOUBLE_EQ(lexer.getNumVal(), -123);
    EXPECT_EQ(lexer.advance(), tok_number);
    EXPECT_DOUBLE_EQ(lexer.getNumVal(), -4);
}

TEST(LexerTest, RecognizesNumberWithLeadingZero) {
    std::istringstream iss("0123");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_number);
    EXPECT_DOUBLE_EQ(lexer.getNumVal(), 123.0);
}

TEST(LexerTest, RecognizesNumberWithDecimal) {
    std::istringstream iss("0.123");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_number);
    EXPECT_DOUBLE_EQ(lexer.getNumVal(), 0.123);
}

TEST(LexerTest, BreaksOnExtraDecimal) {
    std::istringstream iss("0.123.456");
    Lexer lexer(iss);
    EXPECT_DEATH(lexer.advance(), "multiple decimals");
}

TEST(LexerTest, RecognizesSingleCharToken) {
    std::istringstream iss("+");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), '+');
}

TEST(LexerTest, SkipsWhitespace) {
    std::istringstream iss("   def   ");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_def);
}

TEST(LexerTest, SkipsComments) {
    std::istringstream iss("# this is a comment\ndef");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_def);
}

TEST(LexerTest, HandlesMultipleTokens) {
    std::istringstream iss("def foo 42 + extern");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_def);
    EXPECT_EQ(lexer.advance(), tok_identifier);
    EXPECT_EQ(lexer.getIdentifierStr(), "foo");
    EXPECT_EQ(lexer.advance(), tok_number);
    EXPECT_DOUBLE_EQ(lexer.getNumVal(), 42.0);
    EXPECT_EQ(lexer.advance(), '+');
    EXPECT_EQ(lexer.advance(), tok_extern);
    EXPECT_EQ(lexer.advance(), tok_eof);
}

TEST(LexerTest, HandlesIdentifierWithNumbers) {
    std::istringstream iss("foo123");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_identifier);
    EXPECT_EQ(lexer.getIdentifierStr(), "foo123");
}

TEST(LexerTest, HandlesMultipleCommentsAndWhitespace) {
    std::istringstream iss("   # comment1\n# comment2\nextern");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_extern);
}

TEST(LexerTest, GetCurrentTokenAfterGetNextToken) {
    std::istringstream iss("def 123 foo");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_def);
    EXPECT_EQ(lexer.getCurrentToken(), tok_def);
    EXPECT_EQ(lexer.advance(), tok_number);
    EXPECT_EQ(lexer.getCurrentToken(), tok_number);
    EXPECT_EQ(lexer.advance(), tok_identifier);
    EXPECT_EQ(lexer.getCurrentToken(), tok_identifier);
}

TEST(LexerTest, GetCurrentTokenInitiallyEOF) {
    std::istringstream iss("");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.getCurrentToken(), tok_eof);
}

TEST(LexerTest, GetCurrentTokenAfterSingleCharToken) {
    std::istringstream iss("+");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), '+');
    EXPECT_EQ(lexer.getCurrentToken(), '+');
}

TEST(LexerTest, GetCurrentTokenAfterWhitespaceAndComment) {
    std::istringstream iss("   # comment\nextern");
    Lexer lexer(iss);
    lexer.advance();
    EXPECT_EQ(lexer.getCurrentToken(), tok_extern);
}

TEST(LexerTest, GetCurrentTokenAfterMultipleTokens) {
    std::istringstream iss("def foo 42");
    Lexer lexer(iss);
    lexer.advance(); // def
    lexer.advance(); // foo
    lexer.advance(); // 42
    EXPECT_EQ(lexer.getCurrentToken(), tok_number);
}

TEST(LexerTest, GetCurrentTokenAfterEOF) {
    std::istringstream iss("def");
    Lexer lexer(iss);
    lexer.advance(); // def
    EXPECT_EQ(lexer.getCurrentToken(), tok_def);
    lexer.advance(); // eof
    EXPECT_EQ(lexer.getCurrentToken(), tok_eof);
}

TEST(LexerTest, GetCurrentTokenIf) {
    std::istringstream iss("if");
    Lexer lexer(iss);
    lexer.advance(); // def
    EXPECT_EQ(lexer.getCurrentToken(), tok_if);
}

TEST(LexerTest, GetCurrentTokenThen) {
    std::istringstream iss("then");
    Lexer lexer(iss);
    lexer.advance(); // def
    EXPECT_EQ(lexer.getCurrentToken(), tok_then);
}

TEST(LexerTest, GetCurrentTokenElse) {
    std::istringstream iss("else");
    Lexer lexer(iss);
    lexer.advance(); // def
    EXPECT_EQ(lexer.getCurrentToken(), tok_else);
}

TEST(LexerTest, ConsumeAdvancesOnCorrectToken) {
    std::istringstream iss("def extern");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_def);
    lexer.consume(tok_def); // Should not assert
    EXPECT_EQ(lexer.getCurrentToken(), tok_extern);
}

TEST(LexerTest, ConsumeThrowsOnWrongTokenDeathTest) {
    std::istringstream iss("def extern");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_def);
    // Next token is 'extern', so consuming tok_def again should trigger an assert or abort
    // Use death test to check for abnormal termination
    EXPECT_EQ(lexer.advance(), tok_extern);
    ASSERT_DEATH(lexer.consume(tok_def), "");
}

TEST(LexerTest, ConsumeWorksWithSingleCharTokens) {
    std::istringstream iss("+ , ( )");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), '+');
    lexer.consume(Token('+'));
    EXPECT_EQ(lexer.getCurrentToken(), ',');
    lexer.consume(Token(','));
    EXPECT_EQ(lexer.getCurrentToken(), '(');
    lexer.consume(Token('('));
    EXPECT_EQ(lexer.getCurrentToken(), ')');
    lexer.consume(Token(')'));
    EXPECT_EQ(lexer.advance(), tok_eof);
}

TEST(LexerTest, ConsumeWorksWithNumberAndIdentifier) {
    std::istringstream iss("123 foo");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.advance(), tok_number);
    lexer.consume(tok_number);
    EXPECT_EQ(lexer.getCurrentToken(), tok_identifier);
    lexer.consume(tok_identifier);
    EXPECT_EQ(lexer.advance(), tok_eof);
}
