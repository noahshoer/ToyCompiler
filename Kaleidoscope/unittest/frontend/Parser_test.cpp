#include <gtest/gtest.h>
#include <sstream>
#include "frontend/Parser.hpp"

using namespace lang;

TEST(Parser, ParseNumberExpr) {
    std::istringstream input("42");
    Lexer lexer(input);
    lexer.advance();
    Parser parser(lexer);

    auto fcn = parser.parseTopLevelExpr();
    ASSERT_NE(fcn, nullptr);
    EXPECT_EQ(fcn->getName(), "__anon_expr");
    EXPECT_EQ(fcn->getBody()->getType(), "Number");
    EXPECT_EQ(fcn->getBody()->toString(), "42.000000");
}

TEST(Parser, ParseParenExpr) {
    std::istringstream input("(123)");
    Lexer lexer(input);
    lexer.advance();
    Parser parser(lexer);

    auto fcn = parser.parseTopLevelExpr();
    ASSERT_NE(fcn, nullptr);
    EXPECT_EQ(fcn->getName(), "__anon_expr");
    EXPECT_EQ(fcn->getBody()->getType(), "Number")
        << "Expected a NumberExpr inside parentheses";
}

TEST(Parser, ParseVariableExpr) {
    std::istringstream input("foo");
    Lexer lexer(input);
    lexer.advance();
    Parser parser(lexer);

    auto fcn = parser.parseTopLevelExpr();
    ASSERT_NE(fcn, nullptr);
    EXPECT_EQ(fcn->getName(), "__anon_expr");
    EXPECT_EQ(fcn->getBody()->getType(), "Variable");
}

TEST(Parser, ParseCallExprNoArgs) {
    std::istringstream input("bar()");
    Lexer lexer(input);
    lexer.advance();
    Parser parser(lexer);

    auto fcn = parser.parseTopLevelExpr();
    ASSERT_NE(fcn, nullptr);
    EXPECT_EQ(fcn->getName(), "__anon_expr");
    EXPECT_EQ(fcn->getBody()->getType(), "Call");
}

TEST(Parser, ParseCallExprWithArgs) {
    std::istringstream input("baz(1, x, 3.14)");
    Lexer lexer(input);
    lexer.advance();
    Parser parser(lexer);

    auto fcn = parser.parseTopLevelExpr();
    ASSERT_NE(fcn, nullptr);
    EXPECT_EQ(fcn->getName(), "__anon_expr");
    EXPECT_EQ(fcn->getBody()->getType(), "Call");
}

TEST(Parser, ParseBinaryExprPrecedence) {
    std::istringstream input("1 + 2 * 3");
    Lexer lexer(input);
    lexer.advance();
    Parser parser(lexer);

    auto expr = parser.parseTopLevelExpr();
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->getName(), "__anon_expr");
    EXPECT_EQ(expr->getBody()->getType(), "Binary");
}

TEST(Parser, ParseDefinition) {
    std::istringstream input("def foo(x y) x + y");
    Lexer lexer(input);
    lexer.advance();
    Parser parser(lexer);

    auto fcn = parser.parseDefinition();
    ASSERT_NE(fcn, nullptr);
    EXPECT_EQ(fcn->getName(), "foo");
    EXPECT_EQ(fcn->getBody()->getType(), "Binary");
}

TEST(Parser, ParseExtern) {
    std::istringstream input("extern sin(x)");
    Lexer lexer(input);
    lexer.advance();
    Parser parser(lexer);

    auto proto = parser.parseExtern();
    ASSERT_NE(proto, nullptr);
    EXPECT_EQ(proto->getName(), "sin");
}

TEST(Parser, ParseTopLevelExpr) {
    std::istringstream input("1 + 2");
    Lexer lexer(input);
    lexer.advance();
    Parser parser(lexer);

    auto fcn = parser.parseTopLevelExpr();
    ASSERT_NE(fcn, nullptr);
    EXPECT_EQ(fcn->getName(), "__anon_expr");
    EXPECT_EQ(fcn->getBody()->getType(), "Binary");
}

TEST(Parser, ParsePrototypeError) {
    std::istringstream input("123 (a)");
    Lexer lexer(input);
    lexer.advance();
    Parser parser(lexer);

    auto proto = parser.parseExtern();
    EXPECT_EQ(proto, nullptr);
}

TEST(Parser, ParseDefinitionError) {
    std::istringstream input("def 123 (a) a");
    Lexer lexer(input);
    lexer.advance();
    Parser parser(lexer);

    auto fcn = parser.parseDefinition();
    EXPECT_EQ(fcn, nullptr);
}

TEST(Parser, ParseExternError) {
    std::istringstream input("extern 123 (a)");
    Lexer lexer(input);
    lexer.advance();
    Parser parser(lexer);

    auto proto = parser.parseExtern();
    EXPECT_EQ(proto, nullptr);
}

TEST(Parser, ParseUnknownTokenError) {
    std::istringstream input("@");
    Lexer lexer(input);
    lexer.advance();
    Parser parser(lexer);

    auto expr = parser.parseTopLevelExpr();
    EXPECT_EQ(expr, nullptr);
}
