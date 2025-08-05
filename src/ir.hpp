//ir.hpp
//定义数据结构
// src/ir.hpp
#pragma once
#include <string>
#include <vector>

// 操作数：可以是具名变量、临时变量、常量或标签
struct Operand {
    enum Kind { VAR, TEMP, CONST, LABEL, NONE};
    Kind kind = NONE;
    std::string name; // 用于 VAR
    int id;           // 用于 TEMP 和 LABEL
    int value;        // 用于 CONST
};

// 三地址码指令
// 三地址码指令
struct Instruction {
    enum OpCode {
        // 算术运算
        ADD, SUB, MUL, DIV, MOD,

        // 逻辑与关系运算
        NOT, // 一元逻辑非 !
        EQ,  // 等于 ==
        NEQ, // 不等于 !=
        LT,  // 小于 <
        GT,  // 大于 >
        LE,  // 小于等于 <=
        GE,  // 大于等于 >=

        // 赋值
        ASSIGN,

        // 函数
        PARAM,//传递参数
        CALL,
        RET,

        // 分支与标签
        JUMP,           // 无条件跳转
        JUMP_IF_ZERO,   // 如果 arg1 的值为 0 则跳转
        JUMP_IF_NZERO,  // 如果 arg1 的值不为 0 则跳转
        LABEL
    };

    OpCode opcode;
    Operand result;
    Operand arg1;
    Operand arg2;
};

// 基本块 (Basic Block)
struct BasicBlock {
    std::string label;
    std::vector<Instruction> instructions;
};

// 函数的 IR 表示
struct FunctionIR {
    std::string name;
    std::vector<BasicBlock> blocks;
};

// 整个程序的 IR
struct ModuleIR {
    std::vector<FunctionIR> functions;
};