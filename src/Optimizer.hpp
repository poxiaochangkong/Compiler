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

    /**
     * @brief 执行常量折叠优化。
     * @param module IR模块。
     */
    void run_constant_folding(ModuleIR& module);
    bool run_common_subexpression_elimination(ModuleIR& module);

    /**
     * @brief 执行复写传播优化。
     * @param module IR模块。
     * @return 如果本次传播有任何代码被修改，则返回 true。
     */
    bool run_copy_propagation(ModuleIR& module);

    /**
     * @brief 执行死代码消除优化。
     * @param module IR模块。
     * @return 如果本次消除有任何代码被移除，则返回 true。
     */
    bool run_dead_code_elimination(ModuleIR& module);
};

#endif // OPTIMIZER_HPP
