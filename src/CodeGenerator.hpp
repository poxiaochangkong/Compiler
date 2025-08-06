// CodeGenerator.hpp

#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "ir.hpp" // 确保包含了你的 ir.hpp

class CodeGenerator {
public:
    std::string generate(const ModuleIR& module);

private:
    void generate_function(const FunctionIR& func);

    // 【修改点】在这里增加一个 int total_stack_size 参数
    void generate_instruction(const Instruction& instr, const std::map<std::string, int>& offsets, int total_stack_size);

    std::stringstream m_output;
    int m_param_idx;
};