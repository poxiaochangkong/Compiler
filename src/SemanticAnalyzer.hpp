#pragma once

#include "ast.hpp"
#include <stack>
#include <unordered_map>
#include <string>
#include <iostream> // ���ڴ�ӡ������Ϣ

// ���ڴ洢���ţ�����������������Ϣ
struct SymbolInfo {
    TypeKind type;
    // δ��������չ������洢�������������б��Ƿ��ǳ�����
};

// ����������࣬�̳��� Visitor��������� AST ��ִ�м��
class SemanticAnalyzer : public Visitor {
private:
    // ʹ�� vector ��ģ��������ջ��֧�ֲ���
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes;

    // ���ٵ�ǰ�Ƿ���ѭ���ڣ����ڼ�� break/continue
    int loop_depth = 0;

    // ָ��ǰ���ڷ����ĺ��������ڼ�� return ����
    FuncDef* current_function = nullptr;

    // ���ű������� (����ʵ���� .cpp �ļ���)
    void enter_scope();
    void exit_scope();
    bool declare(const std::string& name, SymbolInfo info); // ����һ���·���
    SymbolInfo* lookup(const std::string& name);            // ����һ������

public:
    SemanticAnalyzer() = default;

    // �������
    void analyze(Program* root);

    // --- Visitor �ӿ�ʵ�� ---
    // ��д���� visit ��������������Щ�����ľ���ʵ�ֽ��� SemanticAnalyzer.cpp �����

    // �����뺯��
    void visit(Program* node) override;
    void visit(FuncDef* node) override;
    void visit(Param* node) override;

    // ���
    void visit(Block* node) override;
    void visit(ExprStmt* node) override;
    void visit(AssignStmt* node) override;
    void visit(DeclStmt* node) override;
    void visit(ReturnStmt* node) override;
    void visit(BreakStmt* node) override;
    void visit(ContinueStmt* node) override;
    void visit(IfStmt* node) override;
    void visit(WhileStmt* node) override;

    // ���ʽ
    void visit(IntLiteral* node) override;
    void visit(VarExpr* node) override;
    void visit(BinaryExpr* node) override;
    void visit(UnaryExpr* node) override;
    void visit(CallExpr* node) override;
};