// src/ast.cpp
#include "ast.hpp"

std::vector<std::unique_ptr<Node>> g_arena;  // �� �����ǡ����塱������洢
Program* g_root = nullptr;