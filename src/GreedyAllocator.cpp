#include "GreedyAllocator.hpp"
#include <sstream>
#include <stdexcept>
#include <algorithm>

// 辅助函数 (无改动)
static std::string get_operand_id_ls(const Operand& op) {
    if (op.kind == Operand::VAR) return op.name;
    if (op.kind == Operand::TEMP) return "t" + std::to_string(op.id);
    return "";
}

// 构造函数 (无改动)
GreedyAllocator::GreedyAllocator() {
    m_physical_regs = { "t3", "t4", "t5" };
    m_next_spill_offset = 0;
    m_spill_victim_idx = 0;
}

// 准备函数
void GreedyAllocator::prepare(const FunctionIR& func) {
    m_func = &func;

    m_free_regs.clear();
    m_free_regs.insert(m_physical_regs.begin(), m_physical_regs.end());
    m_reg_map.clear();
    m_reg_owner.clear();
    m_spill_map.clear();
    m_next_spill_offset = 8;
    m_spill_victim_idx = 0;

    std::stringstream ss;
    for (size_t i = 0; i < func.params.size(); ++i) {
        std::string id = func.params[i].name;
        m_next_spill_offset += 4;
        m_spill_map[id] = m_next_spill_offset;

        if (i < 8) {
            ss << "  sw a" << i << ", -" << m_spill_map[id] << "(fp)\n";
        }
        else {
            int caller_stack_offset = (i - 8) * 4;
            ss << "  lw t6, " << caller_stack_offset << "(fp)\n";
            ss << "  sw t6, -" << m_spill_map[id] << "(fp)\n";
        }
    }
    m_param_init_code = ss.str();
}


// loadOperand (无改动)
std::string GreedyAllocator::loadOperand(const Operand& op, const std::string& dest_reg) {
    std::stringstream ss;
    if (op.kind == Operand::CONST) {
        ss << "  li " << dest_reg << ", " << op.value << "\n";
        return ss.str();
    }

    std::string id = get_operand_id_ls(op);
    if (id.empty()) return "# ERROR: Invalid operand for load\n";

    if (m_reg_map.count(id)) {
        std::string current_reg = m_reg_map.at(id);
        if (current_reg != dest_reg) {
            ss << "  mv " << dest_reg << ", " << current_reg << "\n";
        }
        return ss.str();
    }

    if (m_spill_map.count(id)) {
        ss << "  lw " << dest_reg << ", -" << m_spill_map.at(id) << "(fp)\n";
        return ss.str();
    }

    return "# [LOAD] ERROR: operand " + id + " not found\n";
}

// storeOperand (无改动)
std::string GreedyAllocator::storeOperand(const Operand& op, const std::string& src_reg) {
    std::stringstream ss;
    std::string id = get_operand_id_ls(op);
    if (id.empty()) return "# ERROR: Invalid operand for store\n";

    if (m_spill_map.count(id)) {
        ss << "  sw " << src_reg << ", -" << m_spill_map.at(id) << "(fp)\n";
        if (m_reg_map.count(id)) {
            std::string current_reg = m_reg_map.at(id);
            if (current_reg != src_reg) {
                ss << "  mv " << current_reg << ", " << src_reg << "\n";
            }
        }
        return ss.str();
    }

    std::string dest_reg = getRegFor(op, ss, false);
    if (dest_reg != src_reg) {
        ss << "  mv " << dest_reg << ", " << src_reg << "\n";
    }
    return ss.str();
}

// getRegFor (无改动)
std::string GreedyAllocator::getRegFor(const Operand& op, std::stringstream& ss, bool needs_load) {
    std::string id = get_operand_id_ls(op);

    if (m_reg_map.count(id)) {
        return m_reg_map.at(id);
    }

    if (!m_free_regs.empty()) {
        std::string new_reg = *m_free_regs.begin();
        m_free_regs.erase(m_free_regs.begin());

        m_reg_map[id] = new_reg;
        m_reg_owner[new_reg] = id;

        if (needs_load && m_spill_map.count(id)) {
            ss << "  lw " << new_reg << ", -" << m_spill_map.at(id) << "(fp)\n";
        }
        return new_reg;
    }

    std::string reg_to_spill = m_physical_regs[m_spill_victim_idx];
    m_spill_victim_idx = (m_spill_victim_idx + 1) % m_physical_regs.size();

    spillReg(reg_to_spill, ss);

    m_reg_map[id] = reg_to_spill;
    m_reg_owner[reg_to_spill] = id;

    if (needs_load && m_spill_map.count(id)) {
        ss << "  lw " << reg_to_spill << ", -" << m_spill_map.at(id) << "(fp)\n";
    }
    return reg_to_spill;
}

// spillReg (无改动)
void GreedyAllocator::spillReg(const std::string& reg_to_spill, std::stringstream& ss) {
    if (!m_reg_owner.count(reg_to_spill)) {
        return;
    }

    std::string owner_id = m_reg_owner.at(reg_to_spill);

    if (!m_spill_map.count(owner_id)) {
        m_next_spill_offset += 4;
        m_spill_map[owner_id] = m_next_spill_offset;
    }

    ss << "  sw " << reg_to_spill << ", -" << m_spill_map.at(owner_id) << "(fp)\n";
    m_reg_map.erase(owner_id);
    m_reg_owner.erase(reg_to_spill);
}

// 新增：获取当前使用的物理寄存器列表
std::vector<std::string> GreedyAllocator::getCurrentUsedRegs() const {
    std::vector<std::string> used_regs;
    for (const auto& pair : m_reg_owner) {
        used_regs.push_back(pair.first);
    }
    return used_regs;
}


// --- 函数序言和尾声 (无改动) ---
int GreedyAllocator::getTotalStackSize() const {
    int total_stack_size = m_next_spill_offset;
    if (total_stack_size % 16 != 0) {
        total_stack_size += 16 - (total_stack_size % 16);
    }
    return total_stack_size > 0 ? total_stack_size : 16;
}

std::string GreedyAllocator::getPrologue() {
    int total_stack_size = getTotalStackSize();
    std::stringstream ss;
    ss << m_func->name << ":\n";
    ss << "  addi sp, sp, -" << total_stack_size << "\n";
    ss << "  sw ra, " << total_stack_size - 4 << "(sp)\n";
    ss << "  sw fp, " << total_stack_size - 8 << "(sp)\n";
    ss << "  addi fp, sp, " << total_stack_size << "\n";
    if (!m_param_init_code.empty()) {
        ss << m_param_init_code;
    }
    return ss.str();
}

std::string GreedyAllocator::getEpilogueLabel() const {
    return ".L_epilogue_" + m_func->name;
}

std::string GreedyAllocator::getEpilogue() {
    int total_stack_size = getTotalStackSize();
    std::stringstream ss;
    ss << getEpilogueLabel() << ":\n";
    ss << "  lw fp, " << total_stack_size - 8 << "(sp)\n";
    ss << "  lw ra, " << total_stack_size - 4 << "(sp)\n";
    ss << "  addi sp, sp, " << total_stack_size << "\n";
    ss << "  ret\n";
    return ss.str();
}
