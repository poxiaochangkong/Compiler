#pragma once
#include "ir.hpp"
#include <string>

class RegisterAllocator {
public:
    virtual ~RegisterAllocator() = default;
    virtual void prepare(const FunctionIR& func) = 0;
    virtual std::string getPrologue() = 0;
    virtual std::string getEpilogue() = 0;
    virtual std::string loadOperand(const Operand& op, const std::string& dest_reg) = 0;
    virtual std::string storeOperand(const Operand& op, const std::string& src_reg) = 0;
    virtual int getTotalStackSize() const = 0;
    // 【新增】获取函数统一出口的标签名
    virtual std::string getEpilogueLabel() const = 0;
};
