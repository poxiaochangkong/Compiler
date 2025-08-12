#pragma once

#include <sstream>
#include <string>
#include <memory> // For std::unique_ptr
#include "ir.hpp"
#include "RegisterAllocator.hpp" // �����½ӿ�

class CodeGenerator {
public:
    // �ڹ��캯���о���ʹ�����ַ������
    CodeGenerator();

    std::string generate(const ModuleIR& module);

private:
    void generate_function(const FunctionIR& func);
    void generate_instruction(const Instruction& instr);

    // ��������Ϊָ������ṩ�̶�����ʱ�Ĵ���
    const std::string R_ARG1 = "t0";
    const std::string R_ARG2 = "t1";
    const std::string R_RES = "t2";

    std::stringstream m_output;
    int m_param_idx;

    // ����һ���������ӿڵ�����ָ��
    std::unique_ptr<RegisterAllocator> m_allocator;
};
