#pragma once

#include "ast.hpp"
#include "ir.hpp"
#include <map>
#include <vector>
#include <string>

/**
 * @class IRGenerator
 * @brief 使用访问者模式遍历 AST 并生成三地址码（IR）
 *
 * 这个类负责将经过语义分析的 AST 翻译成一个内存中的中间表示（ModuleIR）。
 * 它处理表达式计算、控制流（if/while）的构建以及函数调用。
 */
class IRGenerator : public Visitor {
public:
    /**
     * @brief 生成 IR 的主入口函数
     * @param root AST 的根节点
     * @return 构建完成的整个程序的 ModuleIR
     */
    ModuleIR generate(Program* root);

    // --- 实现 Visitor 接口 ---
    void visit(Program* node) override;
    void visit(FuncDef* node) override;
    void visit(Param* node) override;
    void visit(Block* node) override;
    void visit(ExprStmt* node) override;
    void visit(AssignStmt* node) override;
    void visit(DeclStmt* node) override;
    void visit(ReturnStmt* node) override;
    void visit(BreakStmt* node) override;
    void visit(ContinueStmt* node) override;
    void visit(IfStmt* node) override;
    void visit(WhileStmt* node) override;
    void visit(IntLiteral* node) override;
    void visit(VarExpr* node) override;
    void visit(BinaryExpr* node) override;
    void visit(UnaryExpr* node) override;
    void visit(CallExpr* node) override;

private:
    // --- 状态管理 ---
    ModuleIR m_module;                      // 正在构建的整个程序的 IR
    FunctionIR* current_func = nullptr;   // 指向当前正在处理的函数 IR
    BasicBlock* current_block = nullptr;  // 指向当前正在填充指令的基本块

    // 用于表达式计算后，在父子节点间传递结果操作数
    Operand m_result_op;

    // 栈结构，用于处理嵌套循环中 break 和 continue 的跳转目标
    std::vector<Operand> break_labels;
    std::vector<Operand> continue_labels;

    // --- 辅助函数 ---
    int temp_counter = 0;   // 临时变量的唯一 ID 计数器
    int label_counter = 0;  // 基本块标签的唯一 ID 计数器

    // --- 新增：用于生成唯一变量名的符号表和计数器 ---
    std::vector<std::map<std::string, Operand>> m_scopes;
    int m_var_counter = 0;
    // --- 新增：作用域管理的辅助函数 ---
    void enter_scope();
    void exit_scope();
    Operand declare_variable(const std::string& name);
    Operand lookup_variable(const std::string& name);

    Operand new_temp();      // 创建一个新的临时变量操作数
    Operand new_label_op();  // 创建一个新的标签操作数，用于跳转指令
    BasicBlock* create_block(const std::string& prefix = ".L"); // 创建一个新的、独立的基本块
    void add_block(BasicBlock* bb); // 将一个块添加到当前函数，并设为当前活动块
    void generate_if_chain(IfStmt* node, Operand merge_label);

    // 将 AST 的二元操作符枚举映射到 IR 的 OpCode 枚举
    Instruction::OpCode map_bin_op(BinOp op);
};
