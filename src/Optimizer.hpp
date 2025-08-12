#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include "IRGenerator.hpp" // ���� ModuleIR �Ķ���

/**
 * @class Optimizer
 * @brief ������м��ʾ(IR)�����Ż���
 */
class Optimizer {
public:
    /**
     * @brief ������IRģ��ִ���Ż���
     * @param module ��Ҫ���Ż��� ModuleIR ���󣬻�ֱ����ԭ���޸ġ�
     */
    void optimize(ModuleIR& module);
};

#endif // OPTIMIZER_HPP
