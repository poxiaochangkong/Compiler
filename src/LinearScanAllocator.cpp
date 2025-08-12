#include "LinearScanAllocator.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

// --- 辅助函数 (无改动) ---
static std::string get_operand_id_ls(const Operand& op) {
    if (op.kind == Operand::VAR) return op.name;
    if (op.kind == Operand::TEMP) return "t" + std::to_string(op.id);
    return "";
}

static bool is_variable_or_temp_ls(const Operand& op) {
    return op.kind == Operand::VAR || op.kind == Operand::TEMP;
}

// --- 实现 ---
std::string LinearScanAllocator::getEpilogueLabel() const {
    return ".L_epilogue_" + m_func->name;
}

std::string LinearScanAllocator::getEpilogue() {
    int total_stack_size = getTotalStackSize();
    std::stringstream ss;
    ss << getEpilogueLabel() << ":\n"; // 使用统一的标签
    if (total_stack_size > 0) {
        ss << "  lw fp, " << total_stack_size - 8 << "(sp)\n";
        ss << "  lw ra, " << total_stack_size - 4 << "(sp)\n";
        ss << "  addi sp, sp, " << total_stack_size << "\n";
    }
    ss << "  ret\n";
    return ss.str();
}

LinearScanAllocator::LinearScanAllocator() {
    m_physical_regs = { "t0", "t1", "t2", "t3", "t4", "t5", "t6" };
}

void LinearScanAllocator::prepare(const FunctionIR& func) {
    m_func = &func;
    m_intervals.clear();
    m_reg_map.clear();
    m_spill_map.clear();
    m_stack_size_for_spills = 0;
    m_param_init_code = ""; // 清空参数初始化代码

    if (m_func->blocks.empty()) return;

    // ---------------------- 【核心修复】 ----------------------
    // 步骤 1: 强制为所有函数参数在栈上分配空间，并生成初始化代码
    std::stringstream ss;
    for (size_t i = 0; i < func.params.size(); ++i) {
        // 【错误修复】手动从 ParamInfo 创建一个 Operand 对象来传递
        // 假设 func.params[i] 是 ParamInfo 类型，并且有 .name 成员
        std::string id = get_operand_id_ls({ Operand::VAR, func.params[i].name });
        if (id.empty()) continue;

        // 为参数分配一个溢出槽位
        m_stack_size_for_spills += 4;
        m_spill_map[id] = m_stack_size_for_spills;
        int current_offset = m_spill_map[id] + 8; // +8 是为了越过fp和ra

        if (i < 8) {
            // 前8个参数来自 a0-a7 寄存器，将它们存入当前函数的栈帧
            ss << "  sw a" << i << ", -" << current_offset << "(fp)\n";
        }
        else {
            // 后续参数从调用者的栈帧中加载
            // 函数序言执行后，fp 指向调用前的 sp，所以第9个参数(i=8)在 0(fp)
            int caller_stack_offset = (i - 8) * 4;
            ss << "  lw t6, " << caller_stack_offset << "(fp)\n"; // 使用 t6 作为临时中转
            ss << "  sw t6, -" << current_offset << "(fp)\n";
        }
    }
    m_param_init_code = ss.str();
    // -------------------- 【核心修复结束】 --------------------

    // 步骤 2: 正常计算活跃区间和进行线性扫描分配
    compute_live_intervals();
    linear_scan_allocate(); // (上次修复的分裂逻辑依然有效)
}

// compute_live_intervals 和 linear_scan_allocate 函数保持我们上次修复后的版本，无需改动

void LinearScanAllocator::compute_live_intervals() {
    ControlFlowGraph cfg(*m_func);
    cfg.run_liveness_analysis();
    const auto& nodes = cfg.get_nodes();

    int instr_idx = 0;
    for (const auto& node : nodes) {
        std::set<std::string> live = node.live_out;
        for (int i = node.block->instructions.size() - 1; i >= 0; --i) {
            const auto& instr = node.block->instructions[i];
            int current_instr_idx = instr_idx + i;

            for (const auto& var_id : live) {
                if (m_intervals.find(var_id) == m_intervals.end()) {
                    m_intervals[var_id].var_id = var_id;
                    m_intervals[var_id].start = current_instr_idx;
                    m_intervals[var_id].end = current_instr_idx;
                }
                else {
                    m_intervals[var_id].start = std::min(m_intervals[var_id].start, current_instr_idx);
                    m_intervals[var_id].end = std::max(m_intervals[var_id].end, current_instr_idx);
                }
            }

            if (is_variable_or_temp_ls(instr.result)) {
                live.erase(get_operand_id_ls(instr.result));
            }
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
    std::vector<int> call_locations;
    int instr_idx = 0;
    for (const auto& bb : m_func->blocks) {
        for (const auto& instr : bb.instructions) {
            if (instr.opcode == Instruction::CALL) {
                call_locations.push_back(instr_idx);
            }
            instr_idx++;
        }
    }

    std::vector<LiveInterval> final_intervals;
    for (const auto& pair : m_intervals) {
        const LiveInterval& initial_interval = pair.second;
        int last_split_point = initial_interval.start;

        for (int call_loc : call_locations) {
            if (call_loc > initial_interval.start && call_loc < initial_interval.end) {
                if (last_split_point < call_loc) {
                    final_intervals.push_back({ initial_interval.var_id, last_split_point, call_loc - 1 });
                }
                last_split_point = call_loc + 1;
            }
        }

        if (last_split_point <= initial_interval.end) {
            final_intervals.push_back({ initial_interval.var_id, last_split_point, initial_interval.end });
        }
    }

    std::vector<LiveInterval>& sorted_intervals = final_intervals;
    std::sort(sorted_intervals.begin(), sorted_intervals.end());

    std::list<LiveInterval> active;
    std::set<std::string> free_regs(m_physical_regs.begin(), m_physical_regs.end());

    for (const auto& current : sorted_intervals) {
        for (auto it = active.begin(); it != active.end(); ) {
            if (it->end >= current.start) break;
            free_regs.insert(m_reg_map.at(it->var_id));
            it = active.erase(it);
        }

        if (free_regs.size() > 0) {
            std::string reg = *free_regs.begin();
            free_regs.erase(free_regs.begin());
            m_reg_map[current.var_id] = reg;

            auto insert_pos = std::upper_bound(active.begin(), active.end(), current,
                [](const LiveInterval& a, const LiveInterval& b) { return a.end < b.end; });
            active.insert(insert_pos, current);
        }
        else {
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
    // 【修改】在此处插入参数保存代码
    if (!m_param_init_code.empty()) {
        ss << m_param_init_code;
    }
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
        // 【修改】统一从fp-based的偏移量加载
        return "  lw " + dest_reg + ", -" + std::to_string(m_spill_map.at(id) + 8) + "(fp)\n";
    }
    // 【修改】移除原有的参数查找循环，因为所有参数都已在 m_spill_map 中
    return "  # [LOAD] ERROR: operand " + id + " not found\n";
}

std::string LinearScanAllocator::storeOperand(const Operand& op, const std::string& src_reg) {
    std::string id = get_operand_id_ls(op);
    if (m_reg_map.count(id)) {
        if (m_reg_map.at(id) != src_reg) return "  mv " + m_reg_map.at(id) + ", " + src_reg + "\n";
        return "";
    }
    if (m_spill_map.count(id)) {
        // 【修改】统一存到fp-based的偏移量
        return "  sw " + src_reg + ", -" + std::to_string(m_spill_map.at(id) + 8) + "(fp)\n";
    }
    // 【修改】移除原有的参数查找循环
    return "  # [STORE] ERROR: operand " + id + " not found\n";
}
