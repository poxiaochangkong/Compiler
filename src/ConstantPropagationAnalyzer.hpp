#ifndef CONSTANT_PROPAGATION_ANALYZER_HPP
#define CONSTANT_PROPAGATION_ANALYZER_HPP

#include "ir.hpp"
#include "ControlFlowGraph.hpp"
#include <map>
#include <string>

// 前向声明
class ControlFlowGraph;
struct FunctionIR;
struct BasicBlock;

// 定义常量状态的数据结构：变量名/临时变量名 -> 常量值
using ConstState = std::map<std::string, int>;

/**
 * @class ConstantPropagationAnalyzer
 * @brief 执行一个函数内的常量传播数据流分析。
 *
 * 这个类使用不动点迭代算法来计算每个基本块入口和出口处的变量常量信息。
 * 它是一个“向前”数据流分析。
 */
class ConstantPropagationAnalyzer {
public:
    // 对给定的函数和其CFG执行常量传播分析
    void run(FunctionIR& func, ControlFlowGraph& cfg);

    // 获取分析结果
    const std::map<const BasicBlock*, ConstState>& get_in_states() const { return m_in_states; }
    const std::map<const BasicBlock*, ConstState>& get_out_states() const { return m_out_states; }

private:
    // 存储每个基本块的IN和OUT状态
    std::map<const BasicBlock*, ConstState> m_in_states;
    std::map<const BasicBlock*, ConstState> m_out_states;
};

#endif // CONSTANT_PROPAGATION_ANALYZER_HPP
