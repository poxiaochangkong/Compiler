//ir.hpp
//定义数据结构
#pragma once
#include <string>
#include <vector>

// 操作数：可以是具名变量、临时变量、常量或标签
struct Operand {
    enum Kind {
        VAR,    // 具名变量 (使用 name)
        TEMP,   // 临时变量 (使用 id)
        CONST,  // 常量 (使用 value)
        LABEL   // 标签 (使用 id)
    };

    Kind kind;

    // 根据 kind 的不同，使用对应的成员
    std::string name; // 用于 VAR
    int id;           // 用于 TEMP 和 LABEL
    int value;        // 用于 CONST
};

// 三地址码指令
struct Instruction {
    enum OpCode {
        // 运算
        ADD, SUB, MUL, DIV, MOD,
        // 赋值
        ASSIGN,
        // 函数调用
        CALL, RET,
        // 分支
        JUMP,           // 无条件跳转
        JUMP_IF_ZERO,   // 如果 arg1 == 0 则跳转
        JUMP_IF_NZERO,  // 如果 arg1 != 0 则跳转
        // 标签
        LABEL
    };

    OpCode opcode;
    Operand result;
    Operand arg1;
    Operand arg2;
};

// 基本块 (Basic Block)：一个没有分支的指令序列
struct BasicBlock {
    std::string label;
    std::vector<Instruction> instructions;
};

// 函数：由多个基本块组成
struct FunctionIR {
    std::string name;
    std::vector<BasicBlock> blocks;
};

// 整个程序的 IR
struct ModuleIR {
    std::vector<FunctionIR> functions;
};