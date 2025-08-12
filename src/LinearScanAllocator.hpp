#ifndef LINEAR_SCAN_ALLOCATOR_HPP
#define LINEAR_SCAN_ALLOCATOR_HPP

#include "RegisterAllocator.hpp"
#include "ControlFlowGraph.hpp"
#include <vector>
#include <string>
#include <map>
#include <set>
#include <list>
#include <algorithm>

// 代表一个变量的活跃区间
struct LiveInterval {
    std::string var_id; // 变量的唯一标识 (e.g., "x_0", "t1")
    int start = -1;
    int end = -1;

    // 比较函数，用于排序
    bool operator<(const LiveInterval& other) const {
        return start < other.start;
    }
};

class LinearScanAllocator : public RegisterAllocator {
public:
    LinearScanAllocator();

    // --- 实现基类接口 ---
    void prepare(const FunctionIR& func) override;
    std::string getPrologue()  override;
    std::string getEpilogue()  override;
    std::string loadOperand(const Operand& op, const std::string& dest_reg) override;
    std::string storeOperand(const Operand& op, const std::string& src_reg) override;
    int getTotalStackSize() const override; // <-- 【核心修复】添加缺失的函数声明

private:
    // --- 算法核心步骤 ---
    void compute_live_intervals();
    void linear_scan_allocate();

    // --- 状态成员 ---
    const FunctionIR* m_func;
    std::map<std::string, LiveInterval> m_intervals;
    std::vector<std::string> m_physical_regs;
    int m_stack_size_for_spills;

    // --- 分配结果 ---
    std::map<std::string, std::string> m_reg_map;
    std::map<std::string, int> m_spill_map;
};

#endif // LINEAR_SCAN_ALLOCATOR_HPP
