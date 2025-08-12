#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include "IRGenerator.hpp" // 引入 ModuleIR 的定义

/**
 * @class Optimizer
 * @brief 负责对中间表示(IR)进行优化。
 */
class Optimizer {
public:
    /**
     * @brief 对整个IR模块执行优化。
     * @param module 需要被优化的 ModuleIR 对象，会直接在原地修改。
     */
    void optimize(ModuleIR& module);
};

#endif // OPTIMIZER_HPP
