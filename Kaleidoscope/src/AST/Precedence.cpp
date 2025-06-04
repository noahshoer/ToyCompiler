#include "AST/Precedence.hpp"

std::unordered_map<char, int> BIN_OP_PRECEDENCE = {
    {'<', 10},
    {'+', 20},
    {'-', 20},
    {'*', 40}
};