// SmartSpillAllocator.hpp
#pragma once

#include "RegisterAllocator.hpp"
#include <map>
#include <string>
#include <vector>
#include <set>

/**
 * @class SmartSpillAllocator
 * @brief һ�����þ�̬������ԵļĴ�����������
 *
 * �÷�����Ϊ�����з�����Ƶ����N��������������Ĵ�������������������ջ�ϡ�
 * һ��������ɣ�������λ�ã��Ĵ�����ջ���ں���ִ���ڼ䱣�ֲ��䡣
 * ���ַ����� SpillEverythingAllocator ����Ч��ͬʱ�ȸ��ӵĶ�̬������Ƹ��򵥿ɿ���
 */
class SmartSpillAllocator : public RegisterAllocator {
public:
    SmartSpillAllocator();

    void prepare(const FunctionIR& func) override;
    std::string getPrologue() override;
    std::string getEpilogue() override;
    std::string loadOperand(const Operand& op, const std::string& destReg) override;
    std::string storeOperand(const Operand& op, const std::string& srcReg) override;
    int getTotalStackSize() const override;
    std::string getEpilogueLabel() const override;

private:
    // �ڲ������ṹ��
    struct LiveRange {
        int start = -1;
        int end = -1;
        int frequency = 0;
    };

    // �ڲ���������
    std::string operandToKey(const Operand& op);
    std::string offsetToString(int offset);
    void calculateLiveRanges(const FunctionIR& func);
    void assignLocations();


    // ��Ա����
    std::string m_func_name;
    int m_total_stack_size = 0;
    std::string m_param_init_code;

    // ���Ķ��� ��չ�ļĴ�����
    // ʹ�� s1-s11 (callee-saved) �� t3-t6 (caller-saved)
    // s0 ������ fp��t0-t2 ����ָ�������е���ʱ����
    const std::vector<std::string> m_physical_registers = {
        "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
        "t3", "t4", "t5", "t6"
    };

    // �������� ���ٴ˺�����ʵ��ʹ������Щ callee-saved �Ĵ���
    std::set<std::string> m_used_callee_saved_regs;

    // ����/��ʱ�Ļ�Ծ��Χ��λ����Ϣ
    std::map<std::string, LiveRange> m_live_ranges;
    std::map<std::string, std::string> m_locations;
    std::map<std::string, int> m_stack_offsets;
};