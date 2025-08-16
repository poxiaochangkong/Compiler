// SpillEverythingAllocator.hpp
#pragma once
#include "RegisterAllocator.hpp"
#include <map>

class SpillEverythingAllocator : public RegisterAllocator {
public:
    void prepare(const FunctionIR& func) override;
    std::string getPrologue() override;
    std::string getEpilogue() override;
    std::string loadOperand(const Operand& op, const std::string& destReg) override;
    std::string storeOperand(const Operand& result, const std::string& srcReg) override;
    int getTotalStackSize() const override;
    std::string getEpilogueLabel() const override; // 实现新接口

private:
    std::string operandToKey(const Operand& op);
    std::string offsetToString(int offset);

    std::string m_func_name;
    std::string m_param_init_code;
    int m_total_stack_size = 0;
    std::map<std::string, int> m_stack_offsets;
};