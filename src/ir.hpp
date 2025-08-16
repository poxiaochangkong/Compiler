//ir.hpp
//�������ݽṹ
// src/ir.hpp
#pragma once
#include <string>
#include <vector>
#include "ast.hpp"
// ���� ast.hpp ֻ��Ϊ��ʹ�� TypeKind ö�٣�
// ���õ��������� ir.hpp ��Ҳ����һ�ݣ��Ա��ֺ�˺�ǰ�˵Ľ��
// 
// �������������Ǿ�����������ʱ�������������ǩ
struct Operand {
    enum Kind { VAR, TEMP, CONST, LABEL, NONE};
    Kind kind = NONE;
    std::string name; // ���� VAR
    int id;           // ���� TEMP �� LABEL
    int value;        // ���� CONST
    bool operator==(const Operand& other) const {
        if (kind != other.kind) {
            return false;
        }
        switch (kind) {
        case CONST:
            return value == other.value;
        case VAR:
            return name == other.name;
        case TEMP:
            return id == other.id;
        case LABEL:
            // �����ǩ��Ψһ���������Ʊ�֤
            return name == other.name;
        case NONE:
            return true; // ����NONE���͵Ĳ�����������ȵ�
        default:
            return false;
        }
    }
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

struct ParamInfo {
    std::string name;
    TypeKind TY_INT; // ��ʱ���� int��Ϊδ����չ����
};

// ������ (Basic Block)
struct BasicBlock {
    std::string label;
    std::vector<Instruction> instructions;
};

// ������ IR ��ʾ
struct FunctionIR {
    std::string name;
    std::vector<ParamInfo> params;
    std::vector<BasicBlock> blocks;
};

// ��������� IR
struct ModuleIR {
    std::vector<FunctionIR> functions;
};