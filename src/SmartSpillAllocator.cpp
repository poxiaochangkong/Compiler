#include "SmartSpillAllocator.hpp"
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

std::string SmartSpillAllocator::offsetToString(int offset) {
    return std::to_string(offset) + "(fp)";
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

void SmartSpillAllocator::assignLocations() {
    m_locations.clear();
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
            m_locations[key] = reg;
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
    m_locations.clear();
    m_stack_offsets.clear();
    m_param_init_code.clear();

    calculateLiveRanges(func);
    assignLocations();

    int current_offset = 0;

    current_offset -= 4;
    m_stack_offsets["<ra>"] = current_offset;
    current_offset -= 4;
    m_stack_offsets["<old_fp>"] = current_offset;

    for (const auto& reg : m_used_callee_saved_regs) {
        current_offset -= 4;
        m_stack_offsets[reg] = current_offset;
    }

    std::set<std::string> param_keys;
    for (const auto& p_name : func.params) param_keys.insert(p_name.name);

    for (const auto& [key, range] : m_live_ranges) {
        if (param_keys.find(key) == param_keys.end() && m_locations.find(key) == m_locations.end()) {
            current_offset -= 4;
            m_stack_offsets[key] = current_offset;
            m_locations[key] = offsetToString(current_offset);
        }
    }

    for (size_t i = 0; i < func.params.size() && i < 8; ++i) {
        const auto& param_name = func.params[i].name;
        if (m_locations.find(param_name) == m_locations.end()) {
            current_offset -= 4;
            m_stack_offsets[param_name] = current_offset;
            m_locations[param_name] = offsetToString(current_offset);
        }
    }

    m_total_stack_size = -current_offset;
    if (m_total_stack_size > 0 && m_total_stack_size % 16 != 0) {
        m_total_stack_size = ((m_total_stack_size / 16) + 1) * 16;
    }

    std::stringstream param_ss;
    for (size_t i = 0; i < func.params.size(); ++i) {
        const auto& param_name = func.params[i].name;
        bool is_in_register = m_locations.count(param_name) && m_locations.at(param_name).find("(fp)") == std::string::npos;

        if (i < 8) {
            if (is_in_register) {
                param_ss << "  mv " << m_locations.at(param_name) << ", a" << i << "\n";
            }
            else {
                param_ss << "  sw a" << i << ", " << m_locations.at(param_name) << "\n";
            }
        }
        else {
            int stack_arg_offset = (i - 8) * 4;
            if (is_in_register) {
                param_ss << "  lw " << m_locations.at(param_name) << ", " << stack_arg_offset << "(fp)\n";
            }
            else {
                m_locations[param_name] = offsetToString(stack_arg_offset);
            }
        }
    }
    m_param_init_code = param_ss.str();
}

std::string SmartSpillAllocator::getPrologue() {
    std::stringstream ss;
    ss << m_func_name << ":\n";

    if (m_total_stack_size > 0) {
        ss << "  addi sp, sp, -" << m_total_stack_size << "\n";
        ss << "  sw ra, " << (m_total_stack_size + m_stack_offsets.at("<ra>")) << "(sp)\n";
        ss << "  sw fp, " << (m_total_stack_size + m_stack_offsets.at("<old_fp>")) << "(sp)\n";

        for (const auto& reg : m_used_callee_saved_regs) {
            ss << "  sw " << reg << ", " << (m_total_stack_size + m_stack_offsets.at(reg)) << "(sp)\n";
        }

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

std::string SmartSpillAllocator::loadOperand(const Operand& op, const std::string& destReg) {
    std::stringstream ss;
    if (op.kind == Operand::CONST) {
        ss << "  li " << destReg << ", " << op.value << "\n";
        return ss.str();
    }

    std::string key = operandToKey(op);
    if (m_locations.count(key)) {
        const std::string& loc = m_locations.at(key);
        if (loc.find("(fp)") != std::string::npos) {
            ss << "  lw " << destReg << ", " << loc << "\n";
        }
        else {
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
    if (m_locations.count(key)) {
        const std::string& loc = m_locations.at(key);
        if (loc.find("(fp)") != std::string::npos) {
            ss << "  sw " << srcReg << ", " << loc << "\n";
        }
        else {
            if (srcReg != loc) {
                ss << "  mv " << loc << ", " << srcReg << "\n";
            }
        }
    }
    return ss.str();
}
