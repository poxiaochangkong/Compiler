#include "LinearScanAllocator.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

// --- �������� ---
static std::string get_operand_id_ls(const Operand& op) {
    if (op.kind == Operand::VAR) return op.name;
    if (op.kind == Operand::TEMP) return "t" + std::to_string(op.id);
    return "";
}

static bool is_variable_or_temp_ls(const Operand& op) {
    return op.kind == Operand::VAR || op.kind == Operand::TEMP;
}

// --- ʵ�� ---

LinearScanAllocator::LinearScanAllocator() {
    m_physical_regs = { "t0", "t1", "t2", "t3", "t4", "t5", "t6" };
}

void LinearScanAllocator::prepare(const FunctionIR& func) {
    m_func = &func;
    m_intervals.clear();
    m_reg_map.clear();
    m_spill_map.clear();
    m_stack_size_for_spills = 0;
    if (m_func->blocks.empty()) return;
    compute_live_intervals();
    linear_scan_allocate();
}

// --- �������޸������ø���ȷ�Ļ�Ծ������㷽�� ---
void LinearScanAllocator::compute_live_intervals() {
    ControlFlowGraph cfg(*m_func);
    cfg.run_liveness_analysis();
    const auto& nodes = cfg.get_nodes();

    int instr_idx = 0;
    for (const auto& node : nodes) {
        // ���ڿ��ڵ�ÿ��ָ���ȷ�����ڸ�ָ����ϵĻ�Ծ����
        std::set<std::string> live = node.live_out;

        // �ӻ���������һ��ָ����ǰ����
        for (int i = node.block->instructions.size() - 1; i >= 0; --i) {
            const auto& instr = node.block->instructions[i];
            int current_instr_idx = instr_idx + i;

            // �������е�ǰ��Ծ�ı��������ǵĻ�Ծ�����������쵽��ǰָ��
            for (const auto& var_id : live) {
                if (m_intervals.find(var_id) == m_intervals.end()) {
                    // ��һ�μ��������������ʼ����������
                    m_intervals[var_id].var_id = var_id;
                    m_intervals[var_id].start = current_instr_idx;
                    m_intervals[var_id].end = current_instr_idx;
                }
                else {
                    // ��չ�������������
                    m_intervals[var_id].start = std::min(m_intervals[var_id].start, current_instr_idx);
                    m_intervals[var_id].end = std::max(m_intervals[var_id].end, current_instr_idx);
                }
            }

            // Ӧ������������: live_in = use U (live_out - def)
            // 1. �� live �������Ƴ�����ǰָ���ı��� (def)
            if (is_variable_or_temp_ls(instr.result)) {
                live.erase(get_operand_id_ls(instr.result));
            }
            // 2. �� live ��������ӱ���ǰָ��ʹ�õı��� (use)
            if (is_variable_or_temp_ls(instr.arg1)) {
                live.insert(get_operand_id_ls(instr.arg1));
            }
            if (is_variable_or_temp_ls(instr.arg2)) {
                live.insert(get_operand_id_ls(instr.arg2));
            }
        }
        instr_idx += node.block->instructions.size();
    }
}


void LinearScanAllocator::linear_scan_allocate() {
    std::vector<LiveInterval> sorted_intervals;
    for (const auto& pair : m_intervals) {
        sorted_intervals.push_back(pair.second);
    }
    std::sort(sorted_intervals.begin(), sorted_intervals.end());

    std::list<LiveInterval> active;
    std::set<std::string> free_regs(m_physical_regs.begin(), m_physical_regs.end());

    for (const auto& current : sorted_intervals) {
        // 1. ��� active �б��ͷ��Ѿ�����������ļĴ���
        for (auto it = active.begin(); it != active.end(); ) {
            if (it->end >= current.start) break;
            free_regs.insert(m_reg_map.at(it->var_id));
            it = active.erase(it);
        }

        // 2. ����Ĵ���
        if (free_regs.size() > 0) {
            std::string reg = *free_regs.begin();
            free_regs.erase(free_regs.begin());
            m_reg_map[current.var_id] = reg;

            auto insert_pos = std::upper_bound(active.begin(), active.end(), current,
                [](const LiveInterval& a, const LiveInterval& b) { return a.end < b.end; });
            active.insert(insert_pos, current);
        }
        else { // 3. ��� (Spill)
            const LiveInterval& spill_candidate = active.back();
            if (spill_candidate.end > current.end) {
                std::string reg = m_reg_map.at(spill_candidate.var_id);
                m_reg_map[current.var_id] = reg;

                m_reg_map.erase(spill_candidate.var_id);
                m_stack_size_for_spills += 4;
                m_spill_map[spill_candidate.var_id] = m_stack_size_for_spills;

                active.pop_back();
                auto insert_pos = std::upper_bound(active.begin(), active.end(), current,
                    [](const LiveInterval& a, const LiveInterval& b) { return a.end < b.end; });
                active.insert(insert_pos, current);
            }
            else {
                m_stack_size_for_spills += 4;
                m_spill_map[current.var_id] = m_stack_size_for_spills;
            }
        }
    }
}

int LinearScanAllocator::getTotalStackSize() const {
    int total_stack_size = 8 + m_stack_size_for_spills;
    if (total_stack_size % 16 != 0) {
        total_stack_size += 16 - (total_stack_size % 16);
    }
    return total_stack_size;
}

std::string LinearScanAllocator::getPrologue() {
    int total_stack_size = getTotalStackSize();
    std::stringstream ss;
    ss << m_func->name << ":\n";
    if (total_stack_size > 0) {
        ss << "  addi sp, sp, -" << total_stack_size << "\n";
        ss << "  sw ra, " << total_stack_size - 4 << "(sp)\n";
        ss << "  sw fp, " << total_stack_size - 8 << "(sp)\n";
        ss << "  addi fp, sp, " << total_stack_size << "\n";
    }
    return ss.str();
}

std::string LinearScanAllocator::getEpilogue() {
    int total_stack_size = getTotalStackSize();
    std::stringstream ss;
    if (total_stack_size > 0) {
        ss << "  lw fp, " << total_stack_size - 8 << "(sp)\n";
        ss << "  lw ra, " << total_stack_size - 4 << "(sp)\n";
        ss << "  addi sp, sp, " << total_stack_size << "\n";
    }
    ss << "  ret\n";
    return ss.str();
}

std::string LinearScanAllocator::loadOperand(const Operand& op, const std::string& dest_reg) {
    if (op.kind == Operand::CONST) {
        return "  li " + dest_reg + ", " + std::to_string(op.value) + "\n";
    }
    std::string id = get_operand_id_ls(op);
    if (m_reg_map.count(id)) {
        if (m_reg_map.at(id) != dest_reg) return "  mv " + dest_reg + ", " + m_reg_map.at(id) + "\n";
        return "";
    }
    if (m_spill_map.count(id)) {
        return "  lw " + dest_reg + ", -" + std::to_string(m_spill_map.at(id) + 8) + "(fp)\n";
    }
    for (size_t i = 0; i < m_func->params.size(); ++i) {
        if (m_func->params[i].name == id) {
            if (i < 8) return "  mv " + dest_reg + ", a" + std::to_string(i) + "\n";
            else return "  lw " + dest_reg + ", " + std::to_string((i - 8) * 4) + "(fp)\n";
        }
    }
    return "  # [LOAD] ERROR: operand " + id + " not found\n";
}

std::string LinearScanAllocator::storeOperand(const Operand& op, const std::string& src_reg) {
    std::string id = get_operand_id_ls(op);
    if (m_reg_map.count(id)) {
        if (m_reg_map.at(id) != src_reg) return "  mv " + m_reg_map.at(id) + ", " + src_reg + "\n";
        return "";
    }
    if (m_spill_map.count(id)) {
        return "  sw " + src_reg + ", -" + std::to_string(m_spill_map.at(id) + 8) + "(fp)\n";
    }
    for (size_t i = 0; i < m_func->params.size(); ++i) {
        if (m_func->params[i].name == id) {
            if (i < 8) return "  mv a" + std::to_string(i) + ", " + src_reg + "\n";
            else return "  sw " + src_reg + ", " + std::to_string((i - 8) * 4) + "(fp)\n";
        }
    }
    return "  # [STORE] ERROR: operand " + id + " not found\n";
}
