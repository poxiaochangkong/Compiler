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

// ����һ�������Ļ�Ծ����
struct LiveInterval {
    std::string var_id; // ������Ψһ��ʶ (e.g., "x_0", "t1")
    int start = -1;
    int end = -1;

    // �ȽϺ�������������
    bool operator<(const LiveInterval& other) const {
        return start < other.start;
    }
};

class LinearScanAllocator : public RegisterAllocator {
public:
    LinearScanAllocator();

    // --- ʵ�ֻ���ӿ� ---
    void prepare(const FunctionIR& func) override;
    std::string getPrologue()  override;
    std::string getEpilogue()  override;
    std::string loadOperand(const Operand& op, const std::string& dest_reg) override;
    std::string storeOperand(const Operand& op, const std::string& src_reg) override;
    int getTotalStackSize() const override; // <-- �������޸������ȱʧ�ĺ�������

private:
    // --- �㷨���Ĳ��� ---
    void compute_live_intervals();
    void linear_scan_allocate();

    // --- ״̬��Ա ---
    const FunctionIR* m_func;
    std::map<std::string, LiveInterval> m_intervals;
    std::vector<std::string> m_physical_regs;
    int m_stack_size_for_spills;

    // --- ������ ---
    std::map<std::string, std::string> m_reg_map;
    std::map<std::string, int> m_spill_map;
};

#endif // LINEAR_SCAN_ALLOCATOR_HPP
