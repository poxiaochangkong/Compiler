#ifndef LIVENESS_ANALYZER_HPP
#define LIVENESS_ANALYZER_HPP

// ǰ������������ֱ�Ӱ���ͷ�ļ�
class ControlFlowGraph;

class LivenessAnalyzer {
public:
    // �Ը�����CFGִ�л�Ծ��������
    void run(ControlFlowGraph& cfg);
};

#endif // LIVENESS_ANALYZER_HPP
