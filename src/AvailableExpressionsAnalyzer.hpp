#ifndef AVAILABLE_EXPRESSIONS_ANALYZER_HPP
#define AVAILABLE_EXPRESSIONS_ANALYZER_HPP

#include "ir.hpp"
#include "ControlFlowGraph.hpp"
#include <map>
#include <set>
#include <string>

// 前向声明
class ControlFlowGraph;
struct FunctionIR;
struct BasicBlock;

// 定义可用表达式集合的数据结构
using AvailableExprsSet = std::set<std::string>;

/**
 * @class AvailableExpressionsAnalyzer
 * @brief 执行一个函数内的可用表达式数据流分析 (用于全局公共子表达式消除)。
 *
 * 这是一个“向前”数据流分析，用于计算在每个程序点上，哪些表达式的值是可用的
 * (即已经被计算过，且其操作数自上次计算后未被修改)。
 */
class AvailableExpressionsAnalyzer {
public:
    // 对给定的函数和其CFG执行可用表达式分析
    void run(FunctionIR& func, ControlFlowGraph& cfg);

    // 获取分析结果
    const std::map<const BasicBlock*, AvailableExprsSet>& get_in_states() const { return m_in_states; }
    const std::map<const BasicBlock*, AvailableExprsSet>& get_out_states() const { return m_out_states; }

private:
    // 存储每个基本块的IN和OUT状态
    std::map<const BasicBlock*, AvailableExprsSet> m_in_states;
    std::map<const BasicBlock*, AvailableExprsSet> m_out_states;
};

#endif // AVAILABLE_EXPRESSIONS_ANALYZER_HPP
