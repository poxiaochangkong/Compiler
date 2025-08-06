#pragma once

#include "ir.hpp"
#include <string>
#include <sstream>
#include <map>
#include <vector>

/**
 * @class CodeGenerator
 * @brief 将内存中的 IR 翻译成 RISC-V 32位汇编代码
 *
 * 负责处理函数栈帧、指令选择和简单的寄存器分配。
 */
class CodeGenerator {
public:
    /**
     * @brief 生成汇编代码的主入口
     * @param module 完整的程序 IR
     * @return 包含所有汇编代码的字符串
     */
    std::string generate(const ModuleIR& module);

private:
    // --- 状态管理 ---
    std::stringstream m_output; // 用于拼接最终的汇编代码字符串
    const FunctionIR* m_current_func = nullptr; // 当前正在处理的函数

    // 变量/临时变量到其在栈上偏移量的映射
    std::map<std::string, int> m_var_offsets;
    int m_current_stack_size = 0; // 当前函数所需的总栈大小
    int m_param_count = 0; // 用于跟踪 PARAM 指令的计数器

    // --- 辅助函数 ---

    // 为一个函数生成汇编代码
    void generate_function(const FunctionIR& func);

    // 为一个基本块生成汇编代码
    void generate_block(const BasicBlock& block);

    // 为单条 IR 指令生成汇编代码
    void generate_instruction(const Instruction& instr);

    // 确保操作数的值被加载到一个寄存器中，并返回该寄存器的名字
    // reg_idx 用于指定使用哪个临时寄存器 (0 for t0, 1 for t1, etc.)
    std::string ensure_in_register(const Operand& op, int reg_idx);

    // 为函数生成序言（prologue）和尾声（epilogue）
    void emit_prologue();
    void emit_epilogue();

    // 在当前函数的栈帧中为变量分配空间
    void alloc_vars(const FunctionIR& func);
    int get_var_offset(const Operand& op);
};
