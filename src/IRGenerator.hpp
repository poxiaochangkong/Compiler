#pragma once

#include "ast.hpp"
#include "ir.hpp"
#include <map>
#include <vector>
#include <string>

/**
 * @class IRGenerator
 * @brief ʹ�÷�����ģʽ���� AST ����������ַ�루IR��
 *
 * ����ฺ�𽫾������������ AST �����һ���ڴ��е��м��ʾ��ModuleIR����
 * ��������ʽ���㡢��������if/while���Ĺ����Լ��������á�
 */
class IRGenerator : public Visitor {
public:
    /**
     * @brief ���� IR ������ں���
     * @param root AST �ĸ��ڵ�
     * @return ������ɵ���������� ModuleIR
     */
    ModuleIR generate(Program* root);

    // --- ʵ�� Visitor �ӿ� ---
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
    // --- ״̬���� ---
    ModuleIR m_module;                      // ���ڹ�������������� IR
    FunctionIR* current_func = nullptr;   // ָ��ǰ���ڴ���ĺ��� IR
    BasicBlock* current_block = nullptr;  // ָ��ǰ�������ָ��Ļ�����

    // ���ڱ��ʽ������ڸ��ӽڵ�䴫�ݽ��������
    Operand m_result_op;

    // ջ�ṹ�����ڴ���Ƕ��ѭ���� break �� continue ����תĿ��
    std::vector<Operand> break_labels;
    std::vector<Operand> continue_labels;

    // --- �������� ---
    int temp_counter = 0;   // ��ʱ������Ψһ ID ������
    int label_counter = 0;  // �������ǩ��Ψһ ID ������

    // --- ��������������Ψһ�������ķ��ű�ͼ����� ---
    std::vector<std::map<std::string, Operand>> m_scopes;
    int m_var_counter = 0;
    // --- ���������������ĸ������� ---
    void enter_scope();
    void exit_scope();
    Operand declare_variable(const std::string& name);
    Operand lookup_variable(const std::string& name);

    Operand new_temp();      // ����һ���µ���ʱ����������
    Operand new_label_op();  // ����һ���µı�ǩ��������������תָ��
    BasicBlock* create_block(const std::string& prefix = ".L"); // ����һ���µġ������Ļ�����
    void add_block(BasicBlock* bb); // ��һ������ӵ���ǰ����������Ϊ��ǰ���
    void generate_if_chain(IfStmt* node, Operand merge_label);

    // �� AST �Ķ�Ԫ������ö��ӳ�䵽 IR �� OpCode ö��
    Instruction::OpCode map_bin_op(BinOp op);
};
