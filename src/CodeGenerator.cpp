#include "CodeGenerator.hpp"
#include "SpillEverythingAllocator.hpp" // Ĭ�ϰ��������ڲ���
#include "LinearScanAllocator.hpp"
#include <iostream>
#include <stdexcept>
#include <map>

// �ڹ��캯����ѡ�����
CodeGenerator::CodeGenerator() {
    // ����Ҫ���������� SpillEverythingAllocator ����֤�ع�����ȷ��
    //m_allocator = std::make_unique<SpillEverythingAllocator>();
    // ����֤ͨ����ֻ��ȡ�������ע�ͼ����л�������ɨ��
    m_allocator = std::make_unique<LinearScanAllocator>();
}

// generate_function ����ֻ�������̿��� (�޸Ķ�)
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
    // ���޸���ȷ��ÿ������ĩβ����һ�����У��Է� epilogue �����ڱ�ǩ����
    m_output << "\n";
    m_output << m_allocator->getEpilogue();
}

// �������ع���generate_instruction �ϸ���ѭ Load-Compute-Store ģʽ
void CodeGenerator::generate_instruction(const Instruction& instr) {
    // ӳ�� OpCode �����ָ��
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
        // --- ��Ԫ����/��ϵ���� ---
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
                        // LE �� GE ��αָ���Ҫת��
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
                        // EQ �� NEQ �����⴦��
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

                         // --- һԪ���� ---
    case Instruction::NOT: { // !a <=> a == 0
        m_output << m_allocator->loadOperand(instr.arg1, R_ARG1);
        m_output << "  seqz " << R_RES << ", " << R_ARG1 << "\n";
        m_output << m_allocator->storeOperand(instr.result, R_RES);
        break;
    }

                         // --- ��ֵ ---
    case Instruction::ASSIGN: {
        m_output << m_allocator->loadOperand(instr.arg1, R_ARG1);
        m_output << m_allocator->storeOperand(instr.result, R_ARG1);
        break;
    }

                            // --- ���������뷵�� ---
    case Instruction::PARAM: {
        // RISC-V ǰ8������ͨ�� a0-a7 ����
        if (m_param_idx < 8) {
            std::string reg_name = "a" + std::to_string(m_param_idx);
            m_output << m_allocator->loadOperand(instr.arg1, reg_name);
        }
        else {
            // ����8���Ĳ���ͨ��ջ����
            m_output << m_allocator->loadOperand(instr.arg1, R_ARG1);
            m_output << "  sw " << R_ARG1 << ", " << (m_param_idx - 8) * 4 << "(sp)\n";
        }
        m_param_idx++;
        break;
    }
    case Instruction::CALL: {
        // �ڵ���ǰ����Ҫ�������е����߱���Ĵ���
        // ��һ���� LinearScanAllocator �ķ��������߼���ʽ���
        // SpillEverythingAllocator ����Ȼ����Ҫ����Ϊ���ж�������ջ��
        m_param_idx = 0; // ���ò���������
        m_output << "  call " << instr.arg1.name << "\n";
        if (instr.result.kind != Operand::NONE) {
            // ����ֵ�� a0������浽���������λ��
            m_output << m_allocator->storeOperand(instr.result, "a0");
        }
        break;
    }
    case Instruction::RET: {
        if (instr.arg1.kind != Operand::NONE) {
            // ������ֵ���ص� a0
            m_output << m_allocator->loadOperand(instr.arg1, "a0");
        }
        // ��ת������ĩβ�� epilogue
        m_output << "  j " << m_allocator->getEpilogueLabel() << "\n";
        break;
    }

                         // --- ��֧���ǩ ---
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
        // Label �����ɴ��룬�� generate_function ����
        break;
    default:
        m_output << "  # Unhandled OpCode: " << instr.opcode << "\n";
        break;
    }
}


//����ں�����ȷ�� main ������������ (�޸Ķ�)
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
