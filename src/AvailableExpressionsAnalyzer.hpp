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

// 【修改】定义可用表达式的数据结构：从表达式ID映射到其结果操作数
using AvailableExprsMap = std::map<std::string, Operand>;

class AvailableExpressionsAnalyzer {
public:
    void run(FunctionIR& func, ControlFlowGraph& cfg);

    // 【修改】返回类型变更为新的Map类型
    const std::map<const BasicBlock*, AvailableExprsMap>& get_in_states() const { return m_in_states; }
    const std::map<const BasicBlock*, AvailableExprsMap>& get_out_states() const { return m_out_states; }

private:
    // 【修改】存储类型变更为新的Map类型
    std::map<const BasicBlock*, AvailableExprsMap> m_in_states;
    std::map<const BasicBlock*, AvailableExprsMap> m_out_states;
};

#endif // AVAILABLE_EXPRESSIONS_ANALYZER_HPP
