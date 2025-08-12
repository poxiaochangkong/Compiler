#ifndef CONTROL_FLOW_GRAPH_HPP
#define CONTROL_FLOW_GRAPH_HPP

#include "ir.hpp"
#include <vector>
#include <map>
#include <set>
#include <string>

// ǰ������
struct FunctionIR;
struct BasicBlock;

// CFG�е�һ���ڵ�
struct CFGNode {
    const BasicBlock* block = nullptr;
    int id = -1;
    std::vector<CFGNode*> preds;
    std::vector<CFGNode*> succs;

    // --- ��Ծ�������������ݽṹ ---
    std::set<std::string> def;
    std::set<std::string> use;
    std::set<std::string> live_in;
    std::set<std::string> live_out;
};

// ������ͼ��
class ControlFlowGraph {
public:
    explicit ControlFlowGraph(const FunctionIR& func);

    // --- ������ִ�л�Ծ���������Ľӿ� ---
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
