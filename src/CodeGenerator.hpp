#pragma once

#include "ir.hpp"
#include <string>
#include <sstream>
#include <map>
#include <vector>

/**
 * @class CodeGenerator
 * @brief ���ڴ��е� IR ����� RISC-V 32λ������
 *
 * ��������ջ֡��ָ��ѡ��ͼ򵥵ļĴ������䡣
 */
class CodeGenerator {
public:
    /**
     * @brief ���ɻ�����������
     * @param module �����ĳ��� IR
     * @return �������л�������ַ���
     */
    std::string generate(const ModuleIR& module);

private:
    // --- ״̬���� ---
    std::stringstream m_output; // ����ƴ�����յĻ������ַ���
    const FunctionIR* m_current_func = nullptr; // ��ǰ���ڴ���ĺ���

    // ����/��ʱ����������ջ��ƫ������ӳ��
    std::map<std::string, int> m_var_offsets;
    int m_current_stack_size = 0; // ��ǰ�����������ջ��С
    int m_param_count = 0; // ���ڸ��� PARAM ָ��ļ�����

    // --- �������� ---

    // Ϊһ���������ɻ�����
    void generate_function(const FunctionIR& func);

    // Ϊһ�����������ɻ�����
    void generate_block(const BasicBlock& block);

    // Ϊ���� IR ָ�����ɻ�����
    void generate_instruction(const Instruction& instr);

    // ȷ����������ֵ�����ص�һ���Ĵ����У������ظüĴ���������
    // reg_idx ����ָ��ʹ���ĸ���ʱ�Ĵ��� (0 for t0, 1 for t1, etc.)
    std::string ensure_in_register(const Operand& op, int reg_idx);

    // Ϊ�����������ԣ�prologue����β����epilogue��
    void emit_prologue();
    void emit_epilogue();

    // �ڵ�ǰ������ջ֡��Ϊ��������ռ�
    void alloc_vars(const FunctionIR& func);
    int get_var_offset(const Operand& op);
};
