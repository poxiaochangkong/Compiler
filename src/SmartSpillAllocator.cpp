// SmartSpillAllocator.cpp
#include "SmartSpillAllocator.hpp"
#include <sstream>
#include <algorithm>
#include <stdexcept>

SmartSpillAllocator::SmartSpillAllocator() : m_total_stack_size(0) {}

std::string SmartSpillAllocator::operandToKey(const Operand& op) {
    if (op.kind == Operand::VAR) return op.name;
    if (op.kind == Operand::TEMP) return "t" + std::to_string(op.id);
    return "";
}

std::string SmartSpillAllocator::offsetToString(int offset) {
    return std::to_string(offset) + "(fp)";
}

std::string SmartSpillAllocator::getEpilogueLabel() const {
    return ".L_epilogue_" + m_func_name;
}

int SmartSpillAllocator::getTotalStackSize() const {
    return m_total_stack_size;
}

// ���㺯�������б�������ʱ�����Ļ�Ծ�Ⱥͷ���Ƶ��
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

// ������������Ƶ��Ϊ��������λ�ã��Ĵ�����ջ��
void SmartSpillAllocator::assignLocations() {
    m_locations.clear();
    m_used_callee_saved_regs.clear();

    std::vector<std::string> sorted_vars;
    for (auto const& [key, val] : m_live_ranges) {
        sorted_vars.push_back(key);
    }

    // ��Ƶ�ʽ�������
    std::sort(sorted_vars.begin(), sorted_vars.end(), [&](const std::string& a, const std::string& b) {
        return m_live_ranges.at(a).frequency > m_live_ranges.at(b).frequency;
    });

    std::set<std::string> available_regs(m_physical_registers.begin(), m_physical_registers.end());

    // Ϊ���Ƶ�ı�������Ĵ���
    for (const auto& key : sorted_vars) {
        if (!available_regs.empty()) {
            std::string reg = *available_regs.begin();
            available_regs.erase(available_regs.begin());
            m_locations[key] = reg;

            // ����� callee-saved (s*) �Ĵ���, ��¼����
            if (reg[0] == 's') {
                m_used_callee_saved_regs.insert(reg);
            }
        }
        else {
            // û�п��üĴ����ˣ���Щ�������������ջ
            break;
        }
    }
}


// ���Ķ�����д prepare ��������֧���µķ����߼�
void SmartSpillAllocator::prepare(const FunctionIR& func) {
    m_func_name = func.name;
    m_stack_offsets.clear();
    m_param_init_code.clear();

    // 1. �����Ծ�Ⱥ�Ƶ��
    calculateLiveRanges(func);

    // 2. ����Ĵ�����ջλ�� (ͬʱ��¼��ʹ�õ�s�Ĵ���)
    assignLocations();

    // 3. Ϊ������Ҫ��ջ�Ķ�������ջƫ����
    int current_offset = 0;

    // Ϊ ra �� old_fp ����ռ�, ������ջ֡�����
    current_offset -= 4;
    m_stack_offsets["<ra>"] = current_offset;
    current_offset -= 4;
    m_stack_offsets["<old_fp>"] = current_offset;

    // Ϊ����ʹ�õ��� callee-saved �Ĵ�������ռ�
    for (const auto& reg : m_used_callee_saved_regs) {
        current_offset -= 4;
        m_stack_offsets[reg] = current_offset;
    }

    // 4. Ϊ������������ռ䲢���ɳ�ʼ������
    std::stringstream param_ss;
    for (size_t i = 0; i < func.params.size(); ++i) {
        const auto& param_name = (func.params[i]).name;
        if (i < 8) { // RISC-V ǰ8������ͨ�� a0-a7 ����
            if (m_locations.count(param_name) && m_locations[param_name].find("(fp)") == std::string::npos) {
                // ������������䵽�˼Ĵ���, �� aX �ƶ���ȥ
                param_ss << "  mv " << m_locations[param_name] << ", a" << i << "\n";
            }
            else {
                // ������������, Ϊ�����ջ�ռ䲢����
                current_offset -= 4;
                m_stack_offsets[param_name] = current_offset;
                m_locations[param_name] = offsetToString(current_offset);
                param_ss << "  sw a" << i << ", " << m_locations[param_name] << "\n";
            }
        }
        else {
            // ����8�����������, ���������ݲ�����
            current_offset -= 4;
            m_stack_offsets[param_name] = current_offset;
            m_locations[param_name] = offsetToString(current_offset);
        }
    }
    m_param_init_code = param_ss.str();

    // 5. Ϊ������������ı���/��ʱ��������ջ�ռ�
    for (auto const& [key, val] : m_live_ranges) {
        if (m_locations.find(key) == m_locations.end()) {
            current_offset -= 4;
            m_stack_offsets[key] = current_offset;
            m_locations[key] = offsetToString(current_offset);
        }
    }

    // 6. ������ջ֡��С������
    m_total_stack_size = -current_offset;
    if (m_total_stack_size > 0 && m_total_stack_size % 16 != 0) {
        m_total_stack_size = ((m_total_stack_size / 16) + 1) * 16;
    }
}

// ���Ķ�����д getPrologue ��֧�ֱ��� callee-saved �Ĵ���
std::string SmartSpillAllocator::getPrologue() {
    std::stringstream ss;
    ss << m_func_name << ":\n";

    if (m_total_stack_size > 0) {
        ss << "  addi sp, sp, -" << m_total_stack_size << "\n";
        // ƫ���������fp��, fp=sp+total_size, ���������sp��ƫ���� total_size + fp_offset
        ss << "  sw ra, " << (m_total_stack_size + m_stack_offsets.at("<ra>")) << "(sp)\n";
        ss << "  sw fp, " << (m_total_stack_size + m_stack_offsets.at("<old_fp>")) << "(sp)\n";

        // ���������õ��� callee-saved �Ĵ���
        for (const auto& reg : m_used_callee_saved_regs) {
            ss << "  sw " << reg << ", " << (m_total_stack_size + m_stack_offsets.at(reg)) << "(sp)\n";
        }

        ss << "  addi fp, sp, " << m_total_stack_size << "\n";
    }

    // ���������ʼ������
    if (!m_param_init_code.empty()) {
        ss << m_param_init_code;
    }

    return ss.str();
}

// ���Ķ�����д getEpilogue ��֧�ָֻ� callee-saved �Ĵ���
std::string SmartSpillAllocator::getEpilogue() {
    std::stringstream ss;
    ss << getEpilogueLabel() << ":\n";

    if (m_total_stack_size > 0) {
        // ������� fp ��ƫ�ƴ��ָ��Ĵ���
        // �ָ������õ��� callee-saved �Ĵ���
        for (const auto& reg : m_used_callee_saved_regs) {
            ss << "  lw " << reg << ", " << m_stack_offsets.at(reg) << "(fp)\n";
        }

        ss << "  lw ra, " << m_stack_offsets.at("<ra>") << "(fp)\n";
        ss << "  lw fp, " << m_stack_offsets.at("<old_fp>") << "(fp)\n";
        ss << "  addi sp, sp, " << m_total_stack_size << "\n";
    }

    ss << "  ret\n";
    return ss.str();
}

// loadOperand �� storeOperand ����Ķ�, �������� prepare ���ɵ� m_locations
std::string SmartSpillAllocator::loadOperand(const Operand& op, const std::string& destReg) {
    std::stringstream ss;
    if (op.kind == Operand::CONST) {
        ss << "  li " << destReg << ", " << op.value << "\n";
        return ss.str();
    }

    std::string key = operandToKey(op);
    if (!key.empty() && m_locations.count(key)) {
        const std::string& loc = m_locations.at(key);
        if (loc.find("(fp)") != std::string::npos) {
            // ��ջ��, ִ�� lw
            ss << "  lw " << destReg << ", " << loc << "\n";
        }
        else {
            // �ڼĴ�����, ִ�� mv
            if (destReg != loc) {
                ss << "  mv " << destReg << ", " << loc << "\n";
            }
        }
    }
    return ss.str();
}

std::string SmartSpillAllocator::storeOperand(const Operand& op, const std::string& srcReg) {
    std::stringstream ss;
    std::string key = operandToKey(op);
    if (!key.empty() && m_locations.count(key)) {
        const std::string& loc = m_locations.at(key);
        if (loc.find("(fp)") != std::string::npos) {
            // Ŀ����ջ��, ִ�� sw
            ss << "  sw " << srcReg << ", " << loc << "\n";
        }
        else {
            // Ŀ���ڼĴ�����, ִ�� mv
            if (srcReg != loc) {
                ss << "  mv " << loc << ", " << srcReg << "\n";
            }
        }
    }
    return ss.str();
}