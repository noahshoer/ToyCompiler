#include "mocks/AST/MockNode.hpp"

TEST(MockASTNode, GetTypeInterface) {
    MockASTNode node;

    EXPECT_CALL(node, getType()).Times(1).WillOnce(testing::Return("MockASTNode"));

    const std::string type = node.getType();
    EXPECT_EQ(type, "MockASTNode");
}