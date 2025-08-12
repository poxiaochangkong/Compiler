#include "SpillEverythingAllocator.hpp"
#include <sstream>
#include <algorithm>

std::string SpillEverythingAllocator::operandToKey(const Operand& op) {
    if (op.kind == Operand::VAR) return op.name;
    if (op.kind == Operand::TEMP) return "t" + std::to_string(op.id);
    return "";
}

std::string SpillEverythingAllocator::offsetToString(int offset) {
    return std::to_string(offset) + "(fp)";
}

std::string SpillEverythingAllocator::getEpilogueLabel() const {
    return ".L_epilogue_" + m_func_name;
}

std::string SpillEverythingAllocator::getEpilogue() {
    std::stringstream ss;
    ss << getEpilogueLabel() << ":\n"; // ʹ��ͳһ�ı�ǩ
    if (m_total_stack_size > 0) {
        ss << "  lw ra, " << offsetToString(m_stack_offsets["<ra>"]) << "\n";
        ss << "  lw fp, " << offsetToString(m_stack_offsets["<old_fp>"]) << "\n";
        ss << "  addi sp, sp, " << m_total_stack_size << "\n";
    }
    ss << "  ret\n";
    return ss.str();
}

void SpillEverythingAllocator::prepare(const FunctionIR& func) {
    m_func_name = func.name;
    m_stack_offsets.clear();
    int current_offset = 0;

    // Ϊ ra �� fp ����̶�λ��
    current_offset -= 4;
    const int ra_offset = current_offset;
    m_stack_offsets["<ra>"] = ra_offset; // ʹ���������
    current_offset -= 4;
    const int old_fp_offset = current_offset;
    m_stack_offsets["<old_fp>"] = old_fp_offset;

    // Ϊ���б�������ʱ�������ջ�ռ�
    auto allocate_if_needed = [&](const Operand& op) {
        std::string key = operandToKey(op);
        if (!key.empty() && m_stack_offsets.find(key) == m_stack_offsets.end()) {
            current_offset -= 4;
            m_stack_offsets[key] = current_offset;
        }
    };

    for (const auto& param : func.params) allocate_if_needed({ Operand::VAR, param.name });
    // --- �������룺���������οռ� ---
    int max_outgoing_stack_args = 0;
    int current_call_params = 0;
    for (const auto& bb : func.blocks) {
        for (const auto& instr : bb.instructions) {
            if (instr.opcode == Instruction::PARAM) {
                current_call_params++;
            }
            else if (instr.opcode == Instruction::CALL) {
                if (current_call_params > 8) {
                    int stack_args = current_call_params - 8;
                    if (stack_args > max_outgoing_stack_args) {
                        max_outgoing_stack_args = stack_args;
                    }
                }
                current_call_params = 0; // Ϊ��һ�ε������ü�����
            }
            // --- �����˼���Ϊָ�����Ͳ�������ռ� ---
            allocate_if_needed(instr.result);
            allocate_if_needed(instr.arg1);
            allocate_if_needed(instr.arg2);
        }
    }
    // --- ����������� ---

    // �����οռ�Ҳ������ջ��С����
    current_offset -= max_outgoing_stack_args * 4;

    m_total_stack_size = -current_offset;
    // 16�ֽڶ���
    if (m_total_stack_size % 16 != 0) {
        m_total_stack_size += 16 - (m_total_stack_size % 16);
    }

    std::stringstream ss;
    for (size_t i = 0; i < func.params.size(); ++i) {
        std::string key = operandToKey({ Operand::VAR, func.params[i].name });
        if (m_stack_offsets.count(key)) {
            if (i < 8) {
                // ǰ8���������� a0-a7 �Ĵ����������Ǵ��뵱ǰ������ջ֡
                ss << "  sw a" << i << ", " << offsetToString(m_stack_offsets[key]) << "\n";
            }
            else {
                // ���������ӵ����ߵ�ջ֡�м���
                // ��������ִ�к�fp ָ�����ǰ�� sp
                // ���Ե�9�������� 0(fp)����10���� 4(fp)���Դ�����
                int caller_stack_offset = (i - 8) * 4;
                // 1. �ӵ�����ջ֡���ز����� t0
                ss << "  lw t0, " << caller_stack_offset << "(fp)\n";
                // 2. �� t0 ��ֵ���뵱ǰ����Ϊ�ò��������ջ����
                ss << "  sw t0, " << offsetToString(m_stack_offsets[key]) << "\n";
            }
        }
    }
    m_param_init_code = ss.str();
}

std::string SpillEverythingAllocator::getPrologue() {
    std::stringstream ss;
    ss << m_func_name << ":\n";
    if (m_total_stack_size > 0) {
        ss << "  addi sp, sp, -" << m_total_stack_size << "\n";
        ss << "  sw ra, " << (m_total_stack_size - 4) << "(sp)\n";
        ss << "  sw fp, " << (m_total_stack_size - 8) << "(sp)\n";
        ss << "  addi fp, sp, " << m_total_stack_size << "\n";
    }
    // ��Ӳ�������ָ��
    if (!m_param_init_code.empty()) {
        ss << m_param_init_code;
    }

    return ss.str();
}



std::string SpillEverythingAllocator::loadOperand(const Operand& op, const std::string& destReg) {
    std::stringstream ss;
    if (op.kind == Operand::CONST) {
        ss << "  li " << destReg << ", " << op.value << "\n";
    }
    else {
        std::string key = operandToKey(op);
        if (m_stack_offsets.count(key)) {
            ss << "  lw " << destReg << ", " << offsetToString(m_stack_offsets.at(key)) << "\n";
        }
    }
    return ss.str();
}

std::string SpillEverythingAllocator::storeOperand(const Operand& result, const std::string& srcReg) {
    std::stringstream ss;
    std::string key = operandToKey(result);
    if (m_stack_offsets.count(key)) {
        ss << "  sw " << srcReg << ", " << offsetToString(m_stack_offsets.at(key)) << "\n";
    }
    return ss.str();
}

int SpillEverythingAllocator::getTotalStackSize() const {
    return m_total_stack_size;
}