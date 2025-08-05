//ir.hpp
//�������ݽṹ
#pragma once
#include <string>
#include <vector>

// �������������Ǿ�����������ʱ�������������ǩ
struct Operand {
    enum Kind {
        VAR,    // �������� (ʹ�� name)
        TEMP,   // ��ʱ���� (ʹ�� id)
        CONST,  // ���� (ʹ�� value)
        LABEL   // ��ǩ (ʹ�� id)
    };

    Kind kind;

    // ���� kind �Ĳ�ͬ��ʹ�ö�Ӧ�ĳ�Ա
    std::string name; // ���� VAR
    int id;           // ���� TEMP �� LABEL
    int value;        // ���� CONST
};

// ����ַ��ָ��
struct Instruction {
    enum OpCode {
        // ����
        ADD, SUB, MUL, DIV, MOD,
        // ��ֵ
        ASSIGN,
        // ��������
        CALL, RET,
        // ��֧
        JUMP,           // ��������ת
        JUMP_IF_ZERO,   // ��� arg1 == 0 ����ת
        JUMP_IF_NZERO,  // ��� arg1 != 0 ����ת
        // ��ǩ
        LABEL
    };

    OpCode opcode;
    Operand result;
    Operand arg1;
    Operand arg2;
};

// ������ (Basic Block)��һ��û�з�֧��ָ������
struct BasicBlock {
    std::string label;
    std::vector<Instruction> instructions;
};

// �������ɶ�����������
struct FunctionIR {
    std::string name;
    std::vector<BasicBlock> blocks;
};

// ��������� IR
struct ModuleIR {
    std::vector<FunctionIR> functions;
};