#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include "IRGenerator.hpp" // ���� ModuleIR �Ķ���

/**
 * @class Optimizer
 * @brief ������м��ʾ(IR)�����Ż���
 * ����ͬ���Ż��׶β��Ϊ������˽�к�����
 */
class Optimizer {
public:
    /**
     * @brief ������IRģ��ִ��һϵ���Ż���
     * @param module ��Ҫ���Ż��� ModuleIR ���󣬻�ֱ����ԭ���޸ġ�
     */
    void optimize(ModuleIR& module);

private:
    // --- �����Ż��׶ε�˽��ʵ�� ---

    void run_constant_folding(ModuleIR& module);
    bool run_algebraic_simplification(ModuleIR& module); // <-- ����
    bool run_common_subexpression_elimination(ModuleIR& module);
    bool run_copy_propagation(ModuleIR& module);
    bool run_dead_code_elimination(ModuleIR& module);
};

#endif // OPTIMIZER_HPP
