// src/ast.cpp
#include "ast.hpp"

std::vector<std::unique_ptr<Node>> g_arena;  // ← 这里是“定义”，分配存储
