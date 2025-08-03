// src/ast.cpp
#include "ast.hpp"
#include "SemanticAnalyzer.hpp"

std::vector<std::unique_ptr<Node>> g_arena;  // �� �����ǡ����塱������洢
Program* g_root = nullptr;

// --- ���ʽ (Expressions) accept ����ʵ�� ---

void IntLiteral::accept(Visitor* v) {
    v->visit(this);
}

void VarExpr::accept(Visitor* v) {
    v->visit(this);
}

void BinaryExpr::accept(Visitor* v) {
    v->visit(this);
}

void UnaryExpr::accept(Visitor* v) {
    v->visit(this);
}

void CallExpr::accept(Visitor* v) {
    v->visit(this);
}


// --- ��� (Statements) accept ����ʵ�� ---

void ExprStmt::accept(Visitor* v) {
    v->visit(this);
}

void AssignStmt::accept(Visitor* v) {
    v->visit(this);
}

void DeclStmt::accept(Visitor* v) {
    v->visit(this);
}

void ReturnStmt::accept(Visitor* v) {
    v->visit(this);
}

void BreakStmt::accept(Visitor* v) {
    v->visit(this);
}

void ContinueStmt::accept(Visitor* v) {
    v->visit(this);
}

void Block::accept(Visitor* v) {
    v->visit(this);
}

void IfStmt::accept(Visitor* v) {
    v->visit(this);
}

void WhileStmt::accept(Visitor* v) {
    v->visit(this);
}


// --- ��������� (Functions/Program) accept ����ʵ�� ---

void Param::accept(Visitor* v) {
    v->visit(this);
}

void FuncDef::accept(Visitor* v) {
    v->visit(this);
}

void Program::accept(Visitor* v) {
    v->visit(this);
}
