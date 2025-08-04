#pragma once

#include "ast.hpp"
#include <stack>
#include <unordered_map>
#include <string>
#include <iostream> // 用于打印错误信息

// 用于存储符号（变量、函数）的信息
struct SymbolInfo {
    TypeKind type;
    // 未来可以扩展，例如存储函数参数类型列表、是否是常量等
};

// 语义分析器类，继承自 Visitor，负责遍历 AST 并执行检查
class SemanticAnalyzer : public Visitor {
private:
    // 使用 vector 来模拟作用域栈，支持查找
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes;

    // 跟踪当前是否在循环内，用于检查 break/continue
    int loop_depth = 0;

    // 指向当前正在分析的函数，用于检查 return 类型
    FuncDef* current_function = nullptr;

    // 符号表辅助函数 (具体实现在 .cpp 文件中)
    void enter_scope();
    void exit_scope();
    bool declare(const std::string& name, SymbolInfo info); // 声明一个新符号
    SymbolInfo* lookup(const std::string& name);            // 查找一个符号

public:
    SemanticAnalyzer() = default;

    // 分析入口
    void analyze(Program* root);

    // --- Visitor 接口实现 ---
    // 重写所有 visit 方法的声明，这些方法的具体实现将在 SemanticAnalyzer.cpp 中完成

    // 程序与函数
    void visit(Program* node) override;
    void visit(FuncDef* node) override;
    void visit(Param* node) override;

    // 语句
    void visit(Block* node) override;
    void visit(ExprStmt* node) override;
    void visit(AssignStmt* node) override;
    void visit(DeclStmt* node) override;
    void visit(ReturnStmt* node) override;
    void visit(BreakStmt* node) override;
    void visit(ContinueStmt* node) override;
    void visit(IfStmt* node) override;
    void visit(WhileStmt* node) override;

    // 表达式
    void visit(IntLiteral* node) override;
    void visit(VarExpr* node) override;
    void visit(BinaryExpr* node) override;
    void visit(UnaryExpr* node) override;
    void visit(CallExpr* node) override;
};