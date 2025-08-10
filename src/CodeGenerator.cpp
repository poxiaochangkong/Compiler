#include "CodeGenerator.hpp"
#include "SpillEverythingAllocator.hpp" // ��������ʵ��
#include <iostream>
#include <stdexcept>

// �ڹ��캯����ѡ�����
CodeGenerator::CodeGenerator() {
    // δ���뻻����ʱ��ֻ���޸���һ�У�
    m_allocator = std::make_unique<SpillEverythingAllocator>();
}

// generate_function ����ֻ�������̿���
void CodeGenerator::generate_function(const FunctionIR& func) {
    // 1. ׼���׶�
    m_allocator->prepare(func);

    // 2. ��������
    m_output << m_allocator->getPrologue();

    // 3. ��������ָ��
    for (const auto& bb : func.blocks) {
        if (!bb.label.empty()) {
            m_output << bb.label << ":\n";
        }
        for (const auto& instr : bb.instructions) {
            generate_instruction(instr);
        }
    }
}

// generate_instruction ���ڵ��� m_allocator ������ô�
void CodeGenerator::generate_instruction(const Instruction& instr) {
    switch (instr.opcode) {
    case Instruction::ADD: case Instruction::SUB: case Instruction::MUL: case Instruction::DIV: case Instruction::MOD: {
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << m_allocator->loadOperand(instr.arg2, "t1");
        std::string op_str;
        if (instr.opcode == Instruction::ADD) op_str = "add";
        if (instr.opcode == Instruction::SUB) op_str = "sub";
        if (instr.opcode == Instruction::MUL) op_str = "mul";
        if (instr.opcode == Instruction::DIV) op_str = "div";
        if (instr.opcode == Instruction::MOD) op_str = "rem";
        m_output << "  " << op_str << " t2, t0, t1\n";
        m_output << m_allocator->storeOperand(instr.result, "t2");
        break;
    }
    case Instruction::NOT: { // �߼���
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << "  seqz t0, t0\n"; // ��� t0 Ϊ 0���� t0=1������ t0=0
        m_output << m_allocator->storeOperand(instr.result, "t0");
        break;
    }

    case Instruction::EQ: { // ���� ==
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << m_allocator->loadOperand(instr.arg2, "t1");
        m_output << "  sub t2, t0, t1\n";
        m_output << "  seqz t2, t2\n"; // ��� t2 (��ֵ) Ϊ 0���� t2=1
        m_output << m_allocator->storeOperand(instr.result, "t2");
        break;
    }

    case Instruction::NEQ: { // ������ !=
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << m_allocator->loadOperand(instr.arg2, "t1");
        m_output << "  sub t2, t0, t1\n";
        m_output << "  snez t2, t2\n"; // ��� t2 (��ֵ) ��Ϊ 0���� t2=1
        m_output << m_allocator->storeOperand(instr.result, "t2");
        break;
    }

    case Instruction::LT: { // С�� <
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << m_allocator->loadOperand(instr.arg2, "t1");
        m_output << "  slt t2, t0, t1\n"; // ��� t0 < t1���� t2=1
        m_output << m_allocator->storeOperand(instr.result, "t2");
        break;
    }

    case Instruction::GT: { // ���� >
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << m_allocator->loadOperand(instr.arg2, "t1");
        m_output << "  sgt t2, t0, t1\n"; // ��� t0 > t1���� t2=1 (sgt��αָ��ȼ��� slt t2, t1, t0)
        m_output << m_allocator->storeOperand(instr.result, "t2");
        break;
    }

    case Instruction::LE: { // С�ڵ��� <= (����)
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << m_allocator->loadOperand(instr.arg2, "t1");
        m_output << "  sgt t2, t0, t1\n"; // t2 = (t0 > t1)
        m_output << "  xori t2, t2, 1\n"; // t2 = !t2
        m_output << m_allocator->storeOperand(instr.result, "t2");
        break;
    }

    case Instruction::GE: { // ���ڵ��� >=
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << m_allocator->loadOperand(instr.arg2, "t1");
        m_output << "  slt t2, t0, t1\n"; // t2 = (t0 < t1)
        m_output << "  xori t2, t2, 1\n"; // t2 = !t2
        m_output << m_allocator->storeOperand(instr.result, "t2");
        break;
    }

    case Instruction::JUMP_IF_ZERO: {
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << "  beqz t0, " << instr.arg2.name << "\n";
        break;
    }

    case Instruction::JUMP_IF_NZERO: { // JUMP_IF_NZERO (��Ϊ0����ת)
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << "  bnez t0, " << instr.arg2.name << "\n";
        break;
    case Instruction::JUMP:
        m_output << "  j " << instr.arg1.name << "\n";
        break;
    case Instruction::ASSIGN:
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << m_allocator->storeOperand(instr.result, "t0");
        break;
    case Instruction::RET: {
        if (instr.arg1.kind != Operand::NONE) {
            m_output << m_allocator->loadOperand(instr.arg1, "a0");
        }
        // ֱ�ӻ�ȡβ������
        m_output << m_allocator->getEpilogue();
        break;
    }
    case Instruction::PARAM:
        if (m_param_idx < 8) {
            // ǰ8������ͨ���Ĵ��� a0-a7 ����
            m_output << m_allocator->loadOperand(instr.arg1, "a" + std::to_string(m_param_idx));
        }
        else {
            // ��������ͨ��ջ����
            // 1. ������ֵ���ص���ʱ�Ĵ��� t0
            m_output << m_allocator->loadOperand(instr.arg1, "t0");
            // 2. �� t0 ��ֵ����ջ�У�ƫ�������ݲ�����ż���
            m_output << "  sw t0, " << (m_param_idx - 8) * 4 << "(sp)\n";
        }
        m_param_idx++;
        break;
    case Instruction::CALL:
        m_param_idx = 0; // ���ò���������
        m_output << "  call " << instr.arg1.name << "\n";
        if (instr.result.kind != Operand::NONE) {
            // ������ֵ a0 �浽���������λ��
            m_output << m_allocator->storeOperand(instr.result, "a0");
        }
        break;
    case Instruction::LABEL:
        // Label �����ɴ��룬�� generate_function ����
        break;
    default:
        m_output << "  # Unhandled OpCode: " << instr.opcode << "\n";
        break;
    }
    }
}

//����ں�����ȷ�� main ������������
std::string CodeGenerator::generate(const ModuleIR& module) {
    m_output.str("");
    m_output.clear();
    m_output << ".text\n.globl main\n\n";

    // 1. ���Ҳ��������� main ����
    const FunctionIR* main_func = nullptr;
    for (const auto& func : module.functions) {
        if (func.name == "main") {
            main_func = &func;
            break;
        }
    }

    if (main_func) {
        m_param_idx = 0; // Ϊ main �������ò���������
        generate_function(*main_func);
        m_output << "\n";
    }
    else {
        // ����һ�������Ŀ�ִ�г�����˵��û�� main ��������������
        throw std::runtime_error("CodeGenerator Error: 'main' function not found in module.");
    }

    // 2. �����������з� main ����
    for (const auto& func : module.functions) {
        if (func.name != "main") {
            m_param_idx = 0; // ÿ��������ʼǰ���ò���������
            generate_function(func);
            m_output << "\n";
        }
    }

    return m_output.str();
}