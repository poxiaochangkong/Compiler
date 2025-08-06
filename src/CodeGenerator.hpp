#pragma once

#include <sstream>
#include <string>
#include <memory> // For std::unique_ptr
#include "ir.hpp"
#include "RegisterAllocator.hpp" // 包含新接口

class CodeGenerator {
public:
    // 在构造函数中决定使用哪种分配策略
    CodeGenerator();

    std::string generate(const ModuleIR& module);

private:
    void generate_function(const FunctionIR& func);
    void generate_instruction(const Instruction& instr);

    std::stringstream m_output;
    int m_param_idx;

    // 持有一个分配器接口的智能指针
    std::unique_ptr<RegisterAllocator> m_allocator;
};