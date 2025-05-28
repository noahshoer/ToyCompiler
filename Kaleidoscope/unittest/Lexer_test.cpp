#include "gtest/gtest.h"
#include "frontend/Lexer.hpp"
#include <sstream>

using namespace lang;

TEST(LexerTest, RecognizesEOF) {
    std::istringstream iss("");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.getNextToken(), tok_eof);
}

TEST(LexerTest, RecognizesDefKeyword) {
    std::istringstream iss("def");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.getNextToken(), tok_def);
}

TEST(LexerTest, RecognizesExternKeyword) {
    std::istringstream iss("extern");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.getNextToken(), tok_extern);
}

TEST(LexerTest, RecognizesIdentifier) {
    std::istringstream iss("foo");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.getNextToken(), tok_identifier);
    EXPECT_EQ(lexer.getIdentifierStr(), "foo");
}

TEST(LexerTest, RecognizesNumber) {
    std::istringstream iss("123");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.getNextToken(), tok_number);
    EXPECT_DOUBLE_EQ(lexer.getNumVal(), 123);
}

TEST(LexerTest, DISABLED_RecognizesNumberNegative) {
    std::istringstream iss("-123");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.getNextToken(), tok_number);
    EXPECT_DOUBLE_EQ(lexer.getNumVal(), -123.45);
}

TEST(LexerTest, RecognizesNumberWithLeadingZero) {
    std::istringstream iss("0123");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.getNextToken(), tok_number);
    EXPECT_DOUBLE_EQ(lexer.getNumVal(), 123.0);
}

TEST(LexerTest, RecognizesNumberWithDecimal) {
    std::istringstream iss("0.123");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.getNextToken(), tok_number);
    EXPECT_DOUBLE_EQ(lexer.getNumVal(), 0.123);
}

TEST(LexerTest, DISABLED_BreaksOnExtraDecimal) {
    std::istringstream iss("0.123.456");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.getNextToken(), tok_number);
    EXPECT_DOUBLE_EQ(lexer.getNumVal(), 0.123);
    EXPECT_THROW(lexer.getNextToken(), std::runtime_error);
}

TEST(LexerTest, RecognizesSingleCharToken) {
    std::istringstream iss("+");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.getNextToken(), '+');
}

TEST(LexerTest, SkipsWhitespace) {
    std::istringstream iss("   def   ");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.getNextToken(), tok_def);
}

TEST(LexerTest, SkipsComments) {
    std::istringstream iss("# this is a comment\ndef");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.getNextToken(), tok_def);
}

TEST(LexerTest, HandlesMultipleTokens) {
    std::istringstream iss("def foo 42 + extern");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.getNextToken(), tok_def);
    EXPECT_EQ(lexer.getNextToken(), tok_identifier);
    EXPECT_EQ(lexer.getIdentifierStr(), "foo");
    EXPECT_EQ(lexer.getNextToken(), tok_number);
    EXPECT_DOUBLE_EQ(lexer.getNumVal(), 42.0);
    EXPECT_EQ(lexer.getNextToken(), '+');
    EXPECT_EQ(lexer.getNextToken(), tok_extern);
    EXPECT_EQ(lexer.getNextToken(), tok_eof);
}

TEST(LexerTest, HandlesIdentifierWithNumbers) {
    std::istringstream iss("foo123");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.getNextToken(), tok_identifier);
    EXPECT_EQ(lexer.getIdentifierStr(), "foo123");
}

TEST(LexerTest, HandlesMultipleCommentsAndWhitespace) {
    std::istringstream iss("   # comment1\n# comment2\nextern");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.getNextToken(), tok_extern);
}

TEST(LexerTest, GetCurrentTokenAfterGetNextToken) {
    std::istringstream iss("def 123 foo");
    Lexer lexer(iss);
    EXPECT_EQ(lexer.getNextToken(), tok_def);
    EXPECT_EQ(lexer.getCurrentToken(), tok_def);
    EXPECT_EQ(lexer.getNextToken(), tok_number);
    EXPECT_EQ(lexer.getCurrentToken(), tok_number);
    EXPECT_EQ(lexer.getNextToken(), tok_identifier);
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
    EXPECT_EQ(lexer.getNextToken(), '+');
    EXPECT_EQ(lexer.getCurrentToken(), '+');
}

TEST(LexerTest, GetCurrentTokenAfterWhitespaceAndComment) {
    std::istringstream iss("   # comment\nextern");
    Lexer lexer(iss);
    lexer.getNextToken();
    EXPECT_EQ(lexer.getCurrentToken(), tok_extern);
}

TEST(LexerTest, GetCurrentTokenAfterMultipleTokens) {
    std::istringstream iss("def foo 42");
    Lexer lexer(iss);
    lexer.getNextToken(); // def
    lexer.getNextToken(); // foo
    lexer.getNextToken(); // 42
    EXPECT_EQ(lexer.getCurrentToken(), tok_number);
}

TEST(LexerTest, GetCurrentTokenAfterEOF) {
    std::istringstream iss("def");
    Lexer lexer(iss);
    lexer.getNextToken(); // def
    lexer.getNextToken(); // eof
    EXPECT_EQ(lexer.getCurrentToken(), tok_eof);
}