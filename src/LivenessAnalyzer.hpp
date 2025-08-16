#ifndef LIVENESS_ANALYZER_HPP
#define LIVENESS_ANALYZER_HPP

// 前向声明，避免直接包含头文件
class ControlFlowGraph;

class LivenessAnalyzer {
public:
    // 对给定的CFG执行活跃变量分析
    void run(ControlFlowGraph& cfg);
};

#endif // LIVENESS_ANALYZER_HPP
