#pragma once

#include "RegisterAllocator.hpp"

class SpillEverythingAllocator : public RegisterAllocator {
public:
    void prepare(const FunctionIR& func) override;
    std::string getPrologue() override;
    std::string getEpilogue() override;
    std::string loadOperand(const Operand& op, const std::string& destReg) override;
    std::string storeOperand(const Operand& result, const std::string& srcReg) override;
    int getTotalStackSize() const override;

private:
    std::string operandToKey(const Operand& op);
    std::string offsetToString(int offset);

    std::string m_func_name;
    int m_total_stack_size = 0;
    std::map<std::string, int> m_stack_offsets;
    std::string m_param_init_code;
};