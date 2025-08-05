//ir.hpp
//�������ݽṹ
// src/ir.hpp
#pragma once
#include <string>
#include <vector>

// �������������Ǿ�����������ʱ�������������ǩ
struct Operand {
    enum Kind { VAR, TEMP, CONST, LABEL, NONE};
    Kind kind = NONE;
    std::string name; // ���� VAR
    int id;           // ���� TEMP �� LABEL
    int value;        // ���� CONST
};

// ����ַ��ָ��
// ����ַ��ָ��
struct Instruction {
    enum OpCode {
        // ��������
        ADD, SUB, MUL, DIV, MOD,

        // �߼����ϵ����
        NOT, // һԪ�߼��� !
        EQ,  // ���� ==
        NEQ, // ������ !=
        LT,  // С�� <
        GT,  // ���� >
        LE,  // С�ڵ��� <=
        GE,  // ���ڵ��� >=

        // ��ֵ
        ASSIGN,

        // ����
        PARAM,//���ݲ���
        CALL,
        RET,

        // ��֧���ǩ
        JUMP,           // ��������ת
        JUMP_IF_ZERO,   // ��� arg1 ��ֵΪ 0 ����ת
        JUMP_IF_NZERO,  // ��� arg1 ��ֵ��Ϊ 0 ����ת
        LABEL
    };

    OpCode opcode;
    Operand result;
    Operand arg1;
    Operand arg2;
};

// ������ (Basic Block)
struct BasicBlock {
    std::string label;
    std::vector<Instruction> instructions;
};

// ������ IR ��ʾ
struct FunctionIR {
    std::string name;
    std::vector<BasicBlock> blocks;
};

// ��������� IR
struct ModuleIR {
    std::vector<FunctionIR> functions;
};