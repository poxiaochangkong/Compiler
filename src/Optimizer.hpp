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

    /**
     * @brief ִ�г����۵��Ż���
     * @param module IRģ�顣
     */
    void run_constant_folding(ModuleIR& module);
    bool run_common_subexpression_elimination(ModuleIR& module);

    /**
     * @brief ִ�и�д�����Ż���
     * @param module IRģ�顣
     * @return ������δ������κδ��뱻�޸ģ��򷵻� true��
     */
    bool run_copy_propagation(ModuleIR& module);

    /**
     * @brief ִ�������������Ż���
     * @param module IRģ�顣
     * @return ��������������κδ��뱻�Ƴ����򷵻� true��
     */
    bool run_dead_code_elimination(ModuleIR& module);
};

#endif // OPTIMIZER_HPP
