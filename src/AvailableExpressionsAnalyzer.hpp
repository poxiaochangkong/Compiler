#ifndef AVAILABLE_EXPRESSIONS_ANALYZER_HPP
#define AVAILABLE_EXPRESSIONS_ANALYZER_HPP

#include "ir.hpp"
#include "ControlFlowGraph.hpp"
#include <map>
#include <set>
#include <string>

// ǰ������
class ControlFlowGraph;
struct FunctionIR;
struct BasicBlock;

// ���޸ġ�������ñ��ʽ�����ݽṹ���ӱ��ʽIDӳ�䵽����������
using AvailableExprsMap = std::map<std::string, Operand>;

class AvailableExpressionsAnalyzer {
public:
    void run(FunctionIR& func, ControlFlowGraph& cfg);

    // ���޸ġ��������ͱ��Ϊ�µ�Map����
    const std::map<const BasicBlock*, AvailableExprsMap>& get_in_states() const { return m_in_states; }
    const std::map<const BasicBlock*, AvailableExprsMap>& get_out_states() const { return m_out_states; }

private:
    // ���޸ġ��洢���ͱ��Ϊ�µ�Map����
    std::map<const BasicBlock*, AvailableExprsMap> m_in_states;
    std::map<const BasicBlock*, AvailableExprsMap> m_out_states;
};

#endif // AVAILABLE_EXPRESSIONS_ANALYZER_HPP
