#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include "IRGenerator.hpp"
#include "ConstantPropagationAnalyzer.hpp"
// 【新增】包含可用表达式分析器的头文件
#include "AvailableExpressionsAnalyzer.hpp" 

class Optimizer {
public:
    void optimize(ModuleIR& module);

private:
    // --- 各个优化阶段的私有实现 ---
    bool run_constant_propagation(ModuleIR& module);
    bool run_algebraic_simplification(ModuleIR& module);
    bool run_common_subexpression_elimination(ModuleIR& module);
    bool run_copy_propagation(ModuleIR& module);
    bool run_dead_code_elimination(ModuleIR& module);
    bool run_tail_recursion_elimination(ModuleIR& module);

    // --- 用于在优化器内部创建唯一临时变量的辅助工具 ---
    void initialize_temp_counter(const ModuleIR& module);
    Operand new_temp();
    int m_temp_counter = 0;
};

#endif // OPTIMIZER_HPP
