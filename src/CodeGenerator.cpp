#include "CodeGenerator.hpp"
#include "SpillEverythingAllocator.hpp" // 默认包含，用于测试
#include "LinearScanAllocator.hpp"
#include <iostream>
#include <stdexcept>
#include <map>

// 在构造函数中选择策略
CodeGenerator::CodeGenerator() {
    // 【重要】我们先用 SpillEverythingAllocator 来验证重构的正确性
    //m_allocator = std::make_unique<SpillEverythingAllocator>();
    // 当验证通过后，只需取消下面的注释即可切换到线性扫描
    m_allocator = std::make_unique<LinearScanAllocator>();
}

// generate_function 现在只负责流程控制 (无改动)
void CodeGenerator::generate_function(const FunctionIR& func) {
    m_allocator->prepare(func);
    m_output << m_allocator->getPrologue();

    for (const auto& bb : func.blocks) {
        if (!bb.label.empty()) {
            m_output << bb.label << ":\n";
        }
        for (const auto& instr : bb.instructions) {
            generate_instruction(instr);
        }
    }
    // 【修复】确保每个函数末尾都有一个换行，以防 epilogue 紧跟在标签后面
    m_output << "\n";
    m_output << m_allocator->getEpilogue();
}

// 【核心重构】generate_instruction 严格遵循 Load-Compute-Store 模式
void CodeGenerator::generate_instruction(const Instruction& instr) {
    // 映射 OpCode 到汇编指令
    static const std::map<Instruction::OpCode, std::string> op_to_asm = {
        {Instruction::ADD, "add"}, {Instruction::SUB, "sub"}, {Instruction::MUL, "mul"},
        {Instruction::DIV, "div"}, {Instruction::MOD, "rem"},
        {Instruction::EQ, "seqz"}, // Special: result is `rd = (rs1 == 0)`
        {Instruction::NEQ, "snez"},// Special: result is `rd = (rs1 != 0)`
        {Instruction::LT, "slt"}, {Instruction::GT, "sgt"},
        {Instruction::LE, "sle"},  // Pseudo-instruction
        {Instruction::GE, "sge"}   // Pseudo-instruction
    };

    switch (instr.opcode) {
        // --- 二元算术/关系运算 ---
    case Instruction::ADD:
    case Instruction::SUB:
    case Instruction::MUL:
    case Instruction::DIV:
    case Instruction::MOD:
    case Instruction::LT:
    case Instruction::GT: {
        m_output << m_allocator->loadOperand(instr.arg1, R_ARG1);
        m_output << m_allocator->loadOperand(instr.arg2, R_ARG2);
        m_output << "  " << op_to_asm.at(instr.opcode) << " " << R_RES << ", " << R_ARG1 << ", " << R_ARG2 << "\n";
        m_output << m_allocator->storeOperand(instr.result, R_RES);
        break;
    }
                        // LE 和 GE 是伪指令，需要转换
    case Instruction::LE: { // a <= b  <=> !(b < a)
        m_output << m_allocator->loadOperand(instr.arg2, R_ARG1); // b
        m_output << m_allocator->loadOperand(instr.arg1, R_ARG2); // a
        m_output << "  slt " << R_RES << ", " << R_ARG1 << ", " << R_ARG2 << "\n"; // R_RES = (b < a)
        m_output << "  xori " << R_RES << ", " << R_RES << ", 1\n"; // R_RES = !(b < a)
        m_output << m_allocator->storeOperand(instr.result, R_RES);
        break;
    }
    case Instruction::GE: { // a >= b <=> !(a < b)
        m_output << m_allocator->loadOperand(instr.arg1, R_ARG1); // a
        m_output << m_allocator->loadOperand(instr.arg2, R_ARG2); // b
        m_output << "  slt " << R_RES << ", " << R_ARG1 << ", " << R_ARG2 << "\n"; // R_RES = (a < b)
        m_output << "  xori " << R_RES << ", " << R_RES << ", 1\n"; // R_RES = !(a < b)
        m_output << m_allocator->storeOperand(instr.result, R_RES);
        break;
    }
                        // EQ 和 NEQ 的特殊处理
    case Instruction::EQ: { // a == b <=> (a - b) == 0
        m_output << m_allocator->loadOperand(instr.arg1, R_ARG1);
        m_output << m_allocator->loadOperand(instr.arg2, R_ARG2);
        m_output << "  sub " << R_RES << ", " << R_ARG1 << ", " << R_ARG2 << "\n";
        m_output << "  seqz " << R_RES << ", " << R_RES << "\n";
        m_output << m_allocator->storeOperand(instr.result, R_RES);
        break;
    }
    case Instruction::NEQ: { // a != b <=> (a - b) != 0
        m_output << m_allocator->loadOperand(instr.arg1, R_ARG1);
        m_output << m_allocator->loadOperand(instr.arg2, R_ARG2);
        m_output << "  sub " << R_RES << ", " << R_ARG1 << ", " << R_ARG2 << "\n";
        m_output << "  snez " << R_RES << ", " << R_RES << "\n";
        m_output << m_allocator->storeOperand(instr.result, R_RES);
        break;
    }

                         // --- 一元运算 ---
    case Instruction::NOT: { // !a <=> a == 0
        m_output << m_allocator->loadOperand(instr.arg1, R_ARG1);
        m_output << "  seqz " << R_RES << ", " << R_ARG1 << "\n";
        m_output << m_allocator->storeOperand(instr.result, R_RES);
        break;
    }

                         // --- 赋值 ---
    case Instruction::ASSIGN: {
        m_output << m_allocator->loadOperand(instr.arg1, R_ARG1);
        m_output << m_allocator->storeOperand(instr.result, R_ARG1);
        break;
    }

                            // --- 函数调用与返回 ---
    case Instruction::PARAM: {
        // RISC-V 前8个参数通过 a0-a7 传递
        if (m_param_idx < 8) {
            std::string reg_name = "a" + std::to_string(m_param_idx);
            m_output << m_allocator->loadOperand(instr.arg1, reg_name);
        }
        else {
            // 超出8个的参数通过栈传递
            m_output << m_allocator->loadOperand(instr.arg1, R_ARG1);
            m_output << "  sw " << R_ARG1 << ", " << (m_param_idx - 8) * 4 << "(sp)\n";
        }
        m_param_idx++;
        break;
    }
    case Instruction::CALL: {
        // 在调用前，需要保存所有调用者保存寄存器
        // 这一步由 LinearScanAllocator 的分裂区间逻辑隐式完成
        // SpillEverythingAllocator 则天然不需要，因为所有东西都在栈里
        m_param_idx = 0; // 重置参数计数器
        m_output << "  call " << instr.arg1.name << "\n";
        if (instr.result.kind != Operand::NONE) {
            // 返回值在 a0，将其存到结果变量的位置
            m_output << m_allocator->storeOperand(instr.result, "a0");
        }
        break;
    }
    case Instruction::RET: {
        if (instr.arg1.kind != Operand::NONE) {
            // 将返回值加载到 a0
            m_output << m_allocator->loadOperand(instr.arg1, "a0");
        }
        // 跳转到函数末尾的 epilogue
        m_output << "  j " << m_allocator->getEpilogueLabel() << "\n";
        break;
    }

                         // --- 分支与标签 ---
    case Instruction::JUMP: {
        m_output << "  j " << instr.arg1.name << "\n";
        break;
    }
    case Instruction::JUMP_IF_ZERO: {
        m_output << m_allocator->loadOperand(instr.arg1, R_ARG1);
        m_output << "  beqz " << R_ARG1 << ", " << instr.arg2.name << "\n";
        break;
    }
    case Instruction::JUMP_IF_NZERO: {
        m_output << m_allocator->loadOperand(instr.arg1, R_ARG1);
        m_output << "  bnez " << R_ARG1 << ", " << instr.arg2.name << "\n";
        break;
    }
    case Instruction::LABEL:
        // Label 不生成代码，由 generate_function 处理
        break;
    default:
        m_output << "  # Unhandled OpCode: " << instr.opcode << "\n";
        break;
    }
}


//主入口函数，确保 main 函数最先生成 (无改动)
std::string CodeGenerator::generate(const ModuleIR& module) {
    m_output.str("");
    m_output.clear();
    m_output << ".text\n.globl main\n\n";

    const FunctionIR* main_func = nullptr;
    for (const auto& func : module.functions) {
        if (func.name == "main") {
            main_func = &func;
            break;
        }
    }

    if (main_func) {
        m_param_idx = 0;
        generate_function(*main_func);
        m_output << "\n";
    }

    for (const auto& func : module.functions) {
        if (func.name != "main") {
            m_param_idx = 0;
            generate_function(func);
            m_output << "\n";
        }
    }

    return m_output.str();
}
