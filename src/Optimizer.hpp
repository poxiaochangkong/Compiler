#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include "IRGenerator.hpp" // 引入 ModuleIR 的定义

/**
 * @class Optimizer
 * @brief 负责对中间表示(IR)进行优化。
 * 将不同的优化阶段拆分为独立的私有函数。
 */
class Optimizer {
public:
    /**
     * @brief 对整个IR模块执行一系列优化。
     * @param module 需要被优化的 ModuleIR 对象，会直接在原地修改。
     */
    void optimize(ModuleIR& module);

private:
    // --- 各个优化阶段的私有实现 ---

    void run_constant_folding(ModuleIR& module);
    bool run_algebraic_simplification(ModuleIR& module); // <-- 新增
    bool run_common_subexpression_elimination(ModuleIR& module);
    bool run_copy_propagation(ModuleIR& module);
    bool run_dead_code_elimination(ModuleIR& module);
};

#endif // OPTIMIZER_HPP
