#include "SmartSpillAllocator.hpp"
#include "ir.hpp" // Needed for FunctionIR definition
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <set>

SmartSpillAllocator::SmartSpillAllocator() : m_total_stack_size(0) {}

std::string SmartSpillAllocator::operandToKey(const Operand& op) {
    if (op.kind == Operand::VAR) return op.name;
    if (op.kind == Operand::TEMP) return "t" + std::to_string(op.id);
    return "";
}

std::string SmartSpillAllocator::getEpilogueLabel() const {
    return ".L_epilogue_" + m_func_name;
}

int SmartSpillAllocator::getTotalStackSize() const {
    return m_total_stack_size;
}

void SmartSpillAllocator::calculateLiveRanges(const FunctionIR& func) {
    m_live_ranges.clear();
    int pos = 0;
    for (const auto& bb : func.blocks) {
        for (const auto& instr : bb.instructions) {
            auto update = [&](const Operand& op) {
                std::string key = operandToKey(op);
                if (!key.empty()) {
                    if (m_live_ranges.find(key) == m_live_ranges.end()) {
                        m_live_ranges[key].start = pos;
                    }
                    m_live_ranges[key].end = pos;
                    m_live_ranges[key].frequency++;
                }
            };
            if (instr.result.kind != Operand::NONE) update(instr.result);
            if (instr.arg1.kind != Operand::NONE) update(instr.arg1);
            if (instr.arg2.kind != Operand::NONE) update(instr.arg2);
            pos++;
        }
    }
}

void SmartSpillAllocator::assignRegisters() {
    m_reg_map.clear();
    m_used_callee_saved_regs.clear();

    std::vector<std::pair<std::string, int>> sorted_vars;
    for (auto const& [key, val] : m_live_ranges) {
        sorted_vars.push_back({ key, val.frequency });
    }

    std::sort(sorted_vars.begin(), sorted_vars.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    size_t reg_idx = 0;
    for (const auto& pair : sorted_vars) {
        if (reg_idx < m_physical_registers.size()) {
            const std::string& key = pair.first;
            const std::string& reg = m_physical_registers[reg_idx++];
            m_reg_map[key] = reg;
            if (reg[0] == 's') {
                m_used_callee_saved_regs.insert(reg);
            }
        }
        else {
            break;
        }
    }
}

void SmartSpillAllocator::prepare(const FunctionIR& func) {
    m_func_name = func.name;
    m_current_func = &func;
    m_reg_map.clear();
    m_stack_offsets.clear();
    m_param_init_code.clear();

    calculateLiveRanges(func);
    assignRegisters();

    // 1. ���㱾����������Ŀռ��ƫ����
    int current_offset = 0;

    // Ϊ ra �� fp �������ռ�
    current_offset -= 4;
    m_stack_offsets["<ra>"] = current_offset;
    current_offset -= 4;
    m_stack_offsets["<old_fp>"] = current_offset;

    // Ϊ��Ҫ����� s �Ĵ�������ռ�
    for (const auto& reg : m_used_callee_saved_regs) {
        current_offset -= 4;
        m_stack_offsets[reg] = current_offset;
    }

    // --- FIX START ---
    // Ϊ��Ҫ����ġ��Ĵ������������(a0-a7)����ռ䡣
    // ��ʹ����δʹ�ã�Ҳ����Ϊ�����ռ䣬��Ϊ���ǵ���Լ����һ���֡�
    for (size_t i = 0; i < func.params.size() && i < 8; ++i) {
        const auto& param_name = func.params[i].name;
        if (m_reg_map.find(param_name) == m_reg_map.end()) {
            current_offset -= 4;
            m_stack_offsets[param_name] = current_offset;
        }
    }

    // Ϊ��Ҫ����ġ��ֲ�����/��ʱ����������ռ�
    std::set<std::string> param_names;
    for (const auto& p : func.params) {
        param_names.insert(p.name);
    }
    for (const auto& [key, range] : m_live_ranges) {
        // ������������ ���� ������һ������
        if (m_reg_map.find(key) == m_reg_map.end() && param_names.find(key) == param_names.end()) {
            current_offset -= 4;
            m_stack_offsets[key] = current_offset;
        }
    }
    // --- FIX END ---

    int local_data_size = -current_offset;

    // 2. ����Ϊ����������Outgoing Arguments����������ռ�
    int max_outgoing_args_size = 0;
    for (const auto& bb : func.blocks) {
        for (size_t i = 0; i < bb.instructions.size(); ++i) {
            if (bb.instructions[i].opcode == Instruction::CALL) {
                int param_count = 0;
                for (int j = i - 1; j >= 0; --j) {
                    if (bb.instructions[j].opcode == Instruction::PARAM) {
                        param_count++;
                    }
                    else {
                        break;
                    }
                }
                if (param_count > 8) {
                    int current_args_size = (param_count - 8) * 4;
                    if (current_args_size > max_outgoing_args_size) {
                        max_outgoing_args_size = current_args_size;
                    }
                }
            }
        }
    }

    // 3. ��ջ֡��С = �������ݴ�С + ����������С
    m_total_stack_size = local_data_size + max_outgoing_args_size;

    // 4. ջ֡��С��Ҫ16�ֽڶ���
    if (m_total_stack_size > 0 && m_total_stack_size % 16 != 0) {
        m_total_stack_size = ((m_total_stack_size / 16) + 1) * 16;
    }

    // 5. ���ɲ�����ʼ�����루�ⲿ���߼�����ı䣩
    std::stringstream param_ss;
    for (size_t i = 0; i < func.params.size(); ++i) {
        const auto& param_name = func.params[i].name;

        if (i < 8) { // �Ĵ������ݵĲ���
            if (m_reg_map.count(param_name)) {
                param_ss << "  mv " << m_reg_map.at(param_name) << ", a" << i << "\n";
            }
            else {
                // ����������԰�ȫ�ص��� .at()
                param_ss << "  sw a" << i << ", " << m_stack_offsets.at(param_name) << "(fp)\n";
            }
        }
        else { // ջ���ݵĲ���
            if (m_reg_map.count(param_name)) {
                int stack_arg_offset = (i - 8) * 4;
                param_ss << "  lw " << m_reg_map.at(param_name) << ", " << stack_arg_offset << "(fp)\n";
            }
        }
    }
    m_param_init_code = param_ss.str();
}

std::string SmartSpillAllocator::getPrologue() {
    std::stringstream ss;
    ss << m_func_name << ":\n";

    if (m_total_stack_size > 0) {
        // �ƶ�sp��Ϊ����ջ֡����ռ�
        ss << "  addi sp, sp, -" << m_total_stack_size << "\n";

        // ����ra�;�fp��ջ֡�Ķ���
        // ƫ������������µ�sp
        ss << "  sw ra, " << (m_total_stack_size + m_stack_offsets.at("<ra>")) << "(sp)\n";
        ss << "  sw fp, " << (m_total_stack_size + m_stack_offsets.at("<old_fp>")) << "(sp)\n";

        // ���汻�����߱���ļĴ���
        for (const auto& reg : m_used_callee_saved_regs) {
            // m_stack_offsets �������fp�ĸ�ƫ��
            // (m_total_stack_size + m_stack_offsets.at(reg)) ����ת��Ϊ�����sp����ƫ��
            ss << "  sw " << reg << ", " << (m_total_stack_size + m_stack_offsets.at(reg)) << "(sp)\n";
        }

        // �����µ�fp
        ss << "  addi fp, sp, " << m_total_stack_size << "\n";
    }

    if (!m_param_init_code.empty()) {
        ss << m_param_init_code;
    }

    return ss.str();
}

std::string SmartSpillAllocator::getEpilogue() {
    std::stringstream ss;
    ss << getEpilogueLabel() << ":\n";

    if (m_total_stack_size > 0) {
        // �ָ��������߱���ļĴ���
        for (const auto& reg : m_used_callee_saved_regs) {
            ss << "  lw " << reg << ", " << m_stack_offsets.at(reg) << "(fp)\n";
        }

        // �ָ�ra�;�fp
        ss << "  lw ra, " << m_stack_offsets.at("<ra>") << "(fp)\n";
        ss << "  lw fp, " << m_stack_offsets.at("<old_fp>") << "(fp)\n";

        // �ָ�sp
        ss << "  addi sp, sp, " << m_total_stack_size << "\n";
    }

    ss << "  ret\n";
    return ss.str();
}

std::string SmartSpillAllocator::loadOperand(const Operand& op, const std::string& destReg) {
    std::stringstream ss;
    if (op.kind == Operand::CONST) {
        ss << "  li " << destReg << ", " << op.value << "\n";
        return ss.str();
    }

    std::string key = operandToKey(op);
    if (m_reg_map.count(key)) {
        const std::string& loc_reg = m_reg_map.at(key);
        if (destReg != loc_reg) {
            ss << "  mv " << destReg << ", " << loc_reg << "\n";
        }
    }
    else if (m_stack_offsets.count(key)) {
        ss << "  lw " << destReg << ", " << m_stack_offsets.at(key) << "(fp)\n";
    }
    else {
        // ����ӵ�����ջ֡���صĲ���
        if (m_current_func) {
            for (size_t i = 0; i < m_current_func->params.size(); ++i) {
                if (m_current_func->params[i].name == key) {
                    if (i >= 8) {
                        int stack_arg_offset = (i - 8) * 4;
                        ss << "  lw " << destReg << ", " << stack_arg_offset << "(fp)\n";
                    }
                    break;
                }
            }
        }
    }
    return ss.str();
}

std::string SmartSpillAllocator::storeOperand(const Operand& op, const std::string& srcReg) {
    std::stringstream ss;
    std::string key = operandToKey(op);
    if (m_reg_map.count(key)) {
        const std::string& loc_reg = m_reg_map.at(key);
        if (srcReg != loc_reg) {
            ss << "  mv " << loc_reg << ", " << srcReg << "\n";
        }
    }
    else if (m_stack_offsets.count(key)) {
        ss << "  sw " << srcReg << ", " << m_stack_offsets.at(key) << "(fp)\n";
    }
    return ss.str();
}
