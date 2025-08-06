#pragma once

#include "ir.hpp"
#include <string>
#include <vector>
#include <map>

// 定义一个结构体来表示操作数的位置
// 未来可以扩展，比如增加 Reg(物理寄存器), Imm(立即数) 等
struct OperandLocation {
    enum Kind { STACK };
    Kind kind;
    int offset; // 对于 STACK 类型，表示在 fp 下方的偏移量
};

class RegisterAllocator {
public:
    virtual ~RegisterAllocator() = default;

    // 1. (准备阶段) 分析整个函数，计算栈帧布局等
    virtual void prepare(const FunctionIR& func) = 0;

    // 2. (序言) 获取为函数生成的序言代码
    virtual std::string getPrologue() = 0;

    // 3. (尾声) 获取为函数生成的尾声代码
    virtual std::string getEpilogue() = 0;

    // 4. (加载) 生成将一个操作数加载到指定物理寄存器的代码
    virtual std::string loadOperand(const Operand& op, const std::string& destReg) = 0;

    // 5. (存储) 生成将一个物理寄存器的值存回操作数所在位置的代码
    virtual std::string storeOperand(const Operand& result, const std::string& srcReg) = 0;

    // (可选) 获取总栈帧大小，用于参数传递等
    virtual int getTotalStackSize() const = 0;
};