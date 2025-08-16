#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include "IRGenerator.hpp"
#include "ConstantPropagationAnalyzer.hpp"
// ���������������ñ��ʽ��������ͷ�ļ�
#include "AvailableExpressionsAnalyzer.hpp" 

class Optimizer {
public:
    void optimize(ModuleIR& module);

private:
    // --- �����Ż��׶ε�˽��ʵ�� ---
    bool run_constant_propagation(ModuleIR& module);
    bool run_algebraic_simplification(ModuleIR& module);
    bool run_common_subexpression_elimination(ModuleIR& module);
    bool run_copy_propagation(ModuleIR& module);
    bool run_dead_code_elimination(ModuleIR& module);
    bool run_tail_recursion_elimination(ModuleIR& module);

    // --- �������Ż����ڲ�����Ψһ��ʱ�����ĸ������� ---
    void initialize_temp_counter(const ModuleIR& module);
    Operand new_temp();
    int m_temp_counter = 0;
};

#endif // OPTIMIZER_HPP
