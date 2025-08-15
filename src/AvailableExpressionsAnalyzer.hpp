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

// ������ñ��ʽ���ϵ����ݽṹ
using AvailableExprsSet = std::set<std::string>;

/**
 * @class AvailableExpressionsAnalyzer
 * @brief ִ��һ�������ڵĿ��ñ��ʽ���������� (����ȫ�ֹ����ӱ��ʽ����)��
 *
 * ����һ������ǰ�����������������ڼ�����ÿ��������ϣ���Щ���ʽ��ֵ�ǿ��õ�
 * (���Ѿ����������������������ϴμ����δ���޸�)��
 */
class AvailableExpressionsAnalyzer {
public:
    // �Ը����ĺ�������CFGִ�п��ñ��ʽ����
    void run(FunctionIR& func, ControlFlowGraph& cfg);

    // ��ȡ�������
    const std::map<const BasicBlock*, AvailableExprsSet>& get_in_states() const { return m_in_states; }
    const std::map<const BasicBlock*, AvailableExprsSet>& get_out_states() const { return m_out_states; }

private:
    // �洢ÿ���������IN��OUT״̬
    std::map<const BasicBlock*, AvailableExprsSet> m_in_states;
    std::map<const BasicBlock*, AvailableExprsSet> m_out_states;
};

#endif // AVAILABLE_EXPRESSIONS_ANALYZER_HPP
