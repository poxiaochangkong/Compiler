// SmartSpillAllocator.hpp
#pragma once

#include "RegisterAllocator.hpp"
#include <map>
#include <string>
#include <vector>
#include <set>

/**
 * @class SmartSpillAllocator
 * @brief 一个采用静态分配策略的寄存器分配器。
 *
 * 该分配器为函数中访问最频繁的N个变量分配物理寄存器，其余变量则溢出到栈上。
 * 一旦分配完成，变量的位置（寄存器或栈）在函数执行期间保持不变。
 * 这种方法比 SpillEverythingAllocator 更高效，同时比复杂的动态缓存机制更简单可靠。
 */
class SmartSpillAllocator : public RegisterAllocator {
public:
    SmartSpillAllocator();

    void prepare(const FunctionIR& func) override;
    std::string getPrologue() override;
    std::string getEpilogue() override;
    std::string loadOperand(const Operand& op, const std::string& destReg) override;
    std::string storeOperand(const Operand& op, const std::string& srcReg) override;
    int getTotalStackSize() const override;
    std::string getEpilogueLabel() const override;

private:
    // 内部帮助结构体
    struct LiveRange {
        int start = -1;
        int end = -1;
        int frequency = 0;
    };

    // 内部帮助函数
    std::string operandToKey(const Operand& op);
    std::string offsetToString(int offset);
    void calculateLiveRanges(const FunctionIR& func);
    void assignLocations();


    // 成员变量
    std::string m_func_name;
    int m_total_stack_size = 0;
    std::string m_param_init_code;

    // 【改动】 扩展的寄存器池
    // 使用 s1-s11 (callee-saved) 和 t3-t6 (caller-saved)
    // s0 被用作 fp，t0-t2 用于指令生成中的临时计算
    const std::vector<std::string> m_physical_registers = {
        "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
        "t3", "t4", "t5", "t6"
    };

    // 【新增】 跟踪此函数中实际使用了哪些 callee-saved 寄存器
    std::set<std::string> m_used_callee_saved_regs;

    // 变量/临时的活跃范围与位置信息
    std::map<std::string, LiveRange> m_live_ranges;
    std::map<std::string, std::string> m_locations;
    std::map<std::string, int> m_stack_offsets;
};