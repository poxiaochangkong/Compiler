// CodeGenerator.hpp

#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "ir.hpp" // ȷ����������� ir.hpp

class CodeGenerator {
public:
    std::string generate(const ModuleIR& module);

private:
    void generate_function(const FunctionIR& func);

    // ���޸ĵ㡿����������һ�� int total_stack_size ����
    void generate_instruction(const Instruction& instr, const std::map<std::string, int>& offsets, int total_stack_size);

    std::stringstream m_output;
    int m_param_idx;
};