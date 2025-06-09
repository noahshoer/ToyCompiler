#include "mocks/AST/MockNode.hpp"

TEST(NodeTest, GetTypeInterface) {
    MockASTNode node;

    EXPECT_CALL(node, getType()).Times(1).WillOnce(testing::Return("MockASTNode"));

    const std::string type = node.getType();
    EXPECT_EQ(type, "MockASTNode");
}

TEST(NodeTest, GetSetSourceLoc) {
    MockASTNode node;
    EXPECT_EQ(node.getLine(), 0);
    EXPECT_EQ(node.getCol(), 0);

    node.setSourceLoc(SourceLocation(1, 10));
    EXPECT_EQ(node.getLine(), 1);
    EXPECT_EQ(node.getCol(), 10);
}