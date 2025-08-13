#include "CodeGenerator.hpp"
#include "SpillEverythingAllocator.hpp"
#include "GreedyAllocator.hpp"
#include <iostream>
#include <stdexcept>
#include <map>
#include <vector>

// ���캯�� (�޸Ķ�)
CodeGenerator::CodeGenerator() {
    // ���������Ŀ�ṹ��GreedyAllocator ���ܱ�����Ϊ LinearScanAllocator
    // �ҽ�ʹ�������µ��ϴ��ļ��е����ƣ�GreedyAllocator
    m_allocator = std::make_unique<GreedyAllocator>();
    //m_allocator = std::make_unique<SpillEverythingAllocator>();
}

// generate_function �ع�Ϊ���׶����� (�޸Ķ�)
void CodeGenerator::generate_function(const FunctionIR& func) {
    m_allocator->prepare(func);
    m_param_idx = 0; // Ϊÿ���������ò���������

    // --- �׶� 1: ����������Ļ��������ɵ�һ����ʱ���ַ������� ---
    std::stringstream body_ss;
    for (const auto& bb : func.blocks) {
        if (!bb.label.empty()) {
            body_ss << bb.label << ":\n";
        }
        for (const auto& instr : bb.instructions) {
            std::stringstream temp_instr_ss;
            std::swap(m_output, temp_instr_ss);
            generate_instruction(instr);
            std::swap(m_output, temp_instr_ss);
            body_ss << temp_instr_ss.str();
        }
    }

    // --- �׶� 2: ��װ���յĺ������� ---
    m_output << m_allocator->getPrologue();
    m_output << body_ss.str();
    m_output << "\n";
    m_output << m_allocator->getEpilogue();
}

// generate_instruction �ϸ���ѭ Load-Compute-Store ģʽ�����޸��˺������õ�ջ����
void CodeGenerator::generate_instruction(const Instruction& instr) {
    static const std::map<Instruction::OpCode, std::string> op_to_asm = {
        {Instruction::ADD, "add"}, {Instruction::SUB, "sub"}, {Instruction::MUL, "mul"},
        {Instruction::DIV, "div"}, {Instruction::MOD, "rem"},
        {Instruction::EQ, "seqz"}, {Instruction::NEQ, "snez"},
        {Instruction::LT, "slt"}, {Instruction::GT, "sgt"},
        {Instruction::LE, "sle"}, {Instruction::GE, "sge"}
    };

    GreedyAllocator* greedy_allocator = dynamic_cast<GreedyAllocator*>(m_allocator.get());
    std::vector<std::string> used_regs = greedy_allocator ? greedy_allocator->getCurrentUsedRegs() : std::vector<std::string>();

    switch (instr.opcode) {
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
    case Instruction::LE: {
        m_output << m_allocator->loadOperand(instr.arg2, R_ARG1);
        m_output << m_allocator->loadOperand(instr.arg1, R_ARG2);
        m_output << "  slt " << R_RES << ", " << R_ARG1 << ", " << R_ARG2 << "\n";
        m_output << "  xori " << R_RES << ", " << R_RES << ", 1\n";
        m_output << m_allocator->storeOperand(instr.result, R_RES);
        break;
    }
    case Instruction::GE: {
        m_output << m_allocator->loadOperand(instr.arg1, R_ARG1);
        m_output << m_allocator->loadOperand(instr.arg2, R_ARG2);
        m_output << "  slt " << R_RES << ", " << R_ARG1 << ", " << R_ARG2 << "\n";
        m_output << "  xori " << R_RES << ", " << R_RES << ", 1\n";
        m_output << m_allocator->storeOperand(instr.result, R_RES);
        break;
    }
    case Instruction::EQ: {
        m_output << m_allocator->loadOperand(instr.arg1, R_ARG1);
        m_output << m_allocator->loadOperand(instr.arg2, R_ARG2);
        m_output << "  sub " << R_RES << ", " << R_ARG1 << ", " << R_ARG2 << "\n";
        m_output << "  seqz " << R_RES << ", " << R_RES << "\n";
        m_output << m_allocator->storeOperand(instr.result, R_RES);
        break;
    }
    case Instruction::NEQ: {
        m_output << m_allocator->loadOperand(instr.arg1, R_ARG1);
        m_output << m_allocator->loadOperand(instr.arg2, R_ARG2);
        m_output << "  sub " << R_RES << ", " << R_ARG1 << ", " << R_ARG2 << "\n";
        m_output << "  snez " << R_RES << ", " << R_RES << "\n";
        m_output << m_allocator->storeOperand(instr.result, R_RES);
        break;
    }
    case Instruction::NOT: {
        m_output << m_allocator->loadOperand(instr.arg1, R_ARG1);
        m_output << "  seqz " << R_RES << ", " << R_ARG1 << "\n";
        m_output << m_allocator->storeOperand(instr.result, R_RES);
        break;
    }
    case Instruction::ASSIGN: {
        m_output << m_allocator->loadOperand(instr.arg1, R_ARG1);
        m_output << m_allocator->storeOperand(instr.result, R_ARG1);
        break;
    }
    case Instruction::PARAM: {
        if (m_param_idx < 8) {
            std::string reg_name = "a" + std::to_string(m_param_idx);
            m_output << m_allocator->loadOperand(instr.arg1, reg_name);
        }
        else {
            m_output << m_allocator->loadOperand(instr.arg1, R_ARG1);
            m_output << "  sw " << R_ARG1 << ", " << (m_param_idx - 8) * 4 << "(sp)\n";
        }
        m_param_idx++;
        break;
    }
    case Instruction::CALL: {
        m_output << "  # Save caller-saved registers\n";
        // ��̬������Ҫ����ļĴ�������
        int num_regs_to_save = used_regs.size();
        int stack_size_for_regs = num_regs_to_save * 4;

        // 1. ����ջָ�룬Ϊ����ļĴ����ڳ��ռ�
        if (stack_size_for_regs > 0) {
            m_output << "  addi sp, sp, -" << stack_size_for_regs << "\n";
        }

        // 2. ���Ĵ����е�ֵ�洢���µ�ջ�ռ�
        int current_offset = 0;
        for (const auto& reg : used_regs) {
            m_output << "  sw " << reg << ", " << current_offset << "(sp)\n";
            current_offset += 4;
        }

        // 3. ִ�� CALL ָ��
        m_output << "  call " << instr.arg1.name << "\n";

        // 4. ��ջ�лָ��Ĵ���ֵ
        current_offset = 0;
        for (const auto& reg : used_regs) {
            m_output << "  lw " << reg << ", " << current_offset << "(sp)\n";
            current_offset += 4;
        }

        // 5. �ָ�ջָ��
        if (stack_size_for_regs > 0) {
            m_output << "  addi sp, sp, " << stack_size_for_regs << "\n";
        }

        m_output << "  # End save/restore\n";

        if (instr.result.kind != Operand::NONE) {
            m_output << m_allocator->storeOperand(instr.result, "a0");
        }
        m_param_idx = 0;
        break;
    }
    case Instruction::RET: {
        if (instr.arg1.kind != Operand::NONE) {
            m_output << m_allocator->loadOperand(instr.arg1, "a0");
        }
        m_output << "  j " << m_allocator->getEpilogueLabel() << "\n";
        break;
    }
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
        break;
    default:
        m_output << "  # Unhandled OpCode: " << instr.opcode << "\n";
        break;
    }
}


// generate (����ں���) (�޸Ķ�)
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
