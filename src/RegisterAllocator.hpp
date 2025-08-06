#pragma once

#include "ir.hpp"
#include <string>
#include <vector>
#include <map>

// ����һ���ṹ������ʾ��������λ��
// δ��������չ���������� Reg(����Ĵ���), Imm(������) ��
struct OperandLocation {
    enum Kind { STACK };
    Kind kind;
    int offset; // ���� STACK ���ͣ���ʾ�� fp �·���ƫ����
};

class RegisterAllocator {
public:
    virtual ~RegisterAllocator() = default;

    // 1. (׼���׶�) ������������������ջ֡���ֵ�
    virtual void prepare(const FunctionIR& func) = 0;

    // 2. (����) ��ȡΪ�������ɵ����Դ���
    virtual std::string getPrologue() = 0;

    // 3. (β��) ��ȡΪ�������ɵ�β������
    virtual std::string getEpilogue() = 0;

    // 4. (����) ���ɽ�һ�����������ص�ָ������Ĵ����Ĵ���
    virtual std::string loadOperand(const Operand& op, const std::string& destReg) = 0;

    // 5. (�洢) ���ɽ�һ������Ĵ�����ֵ��ز���������λ�õĴ���
    virtual std::string storeOperand(const Operand& result, const std::string& srcReg) = 0;

    // (��ѡ) ��ȡ��ջ֡��С�����ڲ������ݵ�
    virtual int getTotalStackSize() const = 0;
};