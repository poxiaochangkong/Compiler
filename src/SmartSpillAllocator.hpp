#pragma once

#include "RegisterAllocator.hpp"
#include <map>
#include <string>
#include <vector>
#include <set>

// Forward declaration
struct FunctionIR;

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
    struct LiveRange {
        int start = -1;
        int end = -1;
        int frequency = 0;
    };

    std::string operandToKey(const Operand& op);
    void calculateLiveRanges(const FunctionIR& func);
    void assignRegisters();

    std::string m_func_name;
    int m_total_stack_size = 0;
    std::string m_param_init_code;

    const FunctionIR* m_current_func = nullptr;

    const std::vector<std::string> m_physical_registers = {
        "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11"
    };

    std::set<std::string> m_used_callee_saved_regs;
    std::map<std::string, LiveRange> m_live_ranges;
    std::map<std::string, std::string> m_reg_map; // 变量 -> 寄存器
    std::map<std::string, int> m_stack_offsets;     // 变量 -> 本地栈偏移 (负数)
};