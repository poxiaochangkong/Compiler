#ifndef CONSTANT_PROPAGATION_ANALYZER_HPP
#define CONSTANT_PROPAGATION_ANALYZER_HPP

#include "ir.hpp"
#include "ControlFlowGraph.hpp"
#include <map>
#include <string>

// ǰ������
class ControlFlowGraph;
struct FunctionIR;
struct BasicBlock;

// ���峣��״̬�����ݽṹ��������/��ʱ������ -> ����ֵ
using ConstState = std::map<std::string, int>;

/**
 * @class ConstantPropagationAnalyzer
 * @brief ִ��һ�������ڵĳ�������������������
 *
 * �����ʹ�ò���������㷨������ÿ����������ںͳ��ڴ��ı���������Ϣ��
 * ����һ������ǰ��������������
 */
class ConstantPropagationAnalyzer {
public:
    // �Ը����ĺ�������CFGִ�г�����������
    void run(FunctionIR& func, ControlFlowGraph& cfg);

    // ��ȡ�������
    const std::map<const BasicBlock*, ConstState>& get_in_states() const { return m_in_states; }
    const std::map<const BasicBlock*, ConstState>& get_out_states() const { return m_out_states; }

private:
    // �洢ÿ���������IN��OUT״̬
    std::map<const BasicBlock*, ConstState> m_in_states;
    std::map<const BasicBlock*, ConstState> m_out_states;
};

#endif // CONSTANT_PROPAGATION_ANALYZER_HPP
