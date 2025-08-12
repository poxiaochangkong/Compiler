#ifndef CONTROL_FLOW_GRAPH_HPP
#define CONTROL_FLOW_GRAPH_HPP

#include "ir.hpp"
#include <vector>
#include <map>
#include <set>
#include <string>

// 前向声明
struct FunctionIR;
struct BasicBlock;

// CFG中的一个节点
struct CFGNode {
    const BasicBlock* block = nullptr;
    int id = -1;
    std::vector<CFGNode*> preds;
    std::vector<CFGNode*> succs;

    // --- 活跃变量分析的数据结构 ---
    std::set<std::string> def;
    std::set<std::string> use;
    std::set<std::string> live_in;
    std::set<std::string> live_out;
};

// 控制流图类
class ControlFlowGraph {
public:
    explicit ControlFlowGraph(const FunctionIR& func);

    // --- 新增：执行活跃变量分析的接口 ---
    void run_liveness_analysis();

    const std::vector<CFGNode>& get_nodes() const { return m_nodes; }
    void print_dot(const std::string& func_name) const;

private:
    void build();

    const FunctionIR& m_func;
    std::vector<CFGNode> m_nodes;
    std::map<std::string, CFGNode*> m_label_to_node;
};

#endif // CONTROL_FLOW_GRAPH_HPP
