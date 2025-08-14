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

// 计算函数中所有变量和临时变量的活跃度和访问频率
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

// 【新增】根据频率为变量分配位置（寄存器或栈）
void SmartSpillAllocator::assignLocations() {
    m_locations.clear();
    m_used_callee_saved_regs.clear();

    std::vector<std::string> sorted_vars;
    for (auto const& [key, val] : m_live_ranges) {
        sorted_vars.push_back(key);
    }

    // 按频率降序排序
    std::sort(sorted_vars.begin(), sorted_vars.end(), [&](const std::string& a, const std::string& b) {
        return m_live_ranges.at(a).frequency > m_live_ranges.at(b).frequency;
    });

    std::set<std::string> available_regs(m_physical_registers.begin(), m_physical_registers.end());

    // 为最高频的变量分配寄存器
    for (const auto& key : sorted_vars) {
        if (!available_regs.empty()) {
            std::string reg = *available_regs.begin();
            available_regs.erase(available_regs.begin());
            m_locations[key] = reg;

            // 如果是 callee-saved (s*) 寄存器, 记录下来
            if (reg[0] == 's') {
                m_used_callee_saved_regs.insert(reg);
            }
        }
        else {
            // 没有可用寄存器了，这些变量将被溢出到栈
            break;
        }
    }
}


// 【改动】重写 prepare 函数，以支持新的分配逻辑
void SmartSpillAllocator::prepare(const FunctionIR& func) {
    m_func_name = func.name;
    m_stack_offsets.clear();
    m_param_init_code.clear();

    // 1. 计算活跃度和频率
    calculateLiveRanges(func);

    // 2. 分配寄存器或栈位置 (同时记录了使用的s寄存器)
    assignLocations();

    // 3. 为所有需要上栈的东西计算栈偏移量
    int current_offset = 0;

    // 为 ra 和 old_fp 分配空间, 它们在栈帧的最顶端
    current_offset -= 4;
    m_stack_offsets["<ra>"] = current_offset;
    current_offset -= 4;
    m_stack_offsets["<old_fp>"] = current_offset;

    // 为所有使用到的 callee-saved 寄存器分配空间
    for (const auto& reg : m_used_callee_saved_regs) {
        current_offset -= 4;
        m_stack_offsets[reg] = current_offset;
    }

    // 4. 为函数参数分配空间并生成初始化代码
    std::stringstream param_ss;
    for (size_t i = 0; i < func.params.size(); ++i) {
        const auto& param_name = (func.params[i]).name;
        if (i < 8) { // RISC-V 前8个参数通过 a0-a7 传递
            if (m_locations.count(param_name) && m_locations[param_name].find("(fp)") == std::string::npos) {
                // 如果参数被分配到了寄存器, 从 aX 移动过去
                param_ss << "  mv " << m_locations[param_name] << ", a" << i << "\n";
            }
            else {
                // 如果参数被溢出, 为其分配栈空间并存入
                current_offset -= 4;
                m_stack_offsets[param_name] = current_offset;
                m_locations[param_name] = offsetToString(current_offset);
                param_ss << "  sw a" << i << ", " << m_locations[param_name] << "\n";
            }
        }
        else {
            // 超过8个参数的情况, 本编译器暂不处理
            current_offset -= 4;
            m_stack_offsets[param_name] = current_offset;
            m_locations[param_name] = offsetToString(current_offset);
        }
    }
    m_param_init_code = param_ss.str();

    // 5. 为其他所有溢出的变量/临时变量分配栈空间
    for (auto const& [key, val] : m_live_ranges) {
        if (m_locations.find(key) == m_locations.end()) {
            current_offset -= 4;
            m_stack_offsets[key] = current_offset;
            m_locations[key] = offsetToString(current_offset);
        }
    }

    // 6. 计算总栈帧大小并对齐
    m_total_stack_size = -current_offset;
    if (m_total_stack_size > 0 && m_total_stack_size % 16 != 0) {
        m_total_stack_size = ((m_total_stack_size / 16) + 1) * 16;
    }
}

// 【改动】重写 getPrologue 以支持保存 callee-saved 寄存器
std::string SmartSpillAllocator::getPrologue() {
    std::stringstream ss;
    ss << m_func_name << ":\n";

    if (m_total_stack_size > 0) {
        ss << "  addi sp, sp, -" << m_total_stack_size << "\n";
        // 偏移是相对于fp的, fp=sp+total_size, 所以相对于sp的偏移是 total_size + fp_offset
        ss << "  sw ra, " << (m_total_stack_size + m_stack_offsets.at("<ra>")) << "(sp)\n";
        ss << "  sw fp, " << (m_total_stack_size + m_stack_offsets.at("<old_fp>")) << "(sp)\n";

        // 保存所有用到的 callee-saved 寄存器
        for (const auto& reg : m_used_callee_saved_regs) {
            ss << "  sw " << reg << ", " << (m_total_stack_size + m_stack_offsets.at(reg)) << "(sp)\n";
        }

        ss << "  addi fp, sp, " << m_total_stack_size << "\n";
    }

    // 插入参数初始化代码
    if (!m_param_init_code.empty()) {
        ss << m_param_init_code;
    }

    return ss.str();
}

// 【改动】重写 getEpilogue 以支持恢复 callee-saved 寄存器
std::string SmartSpillAllocator::getEpilogue() {
    std::stringstream ss;
    ss << getEpilogueLabel() << ":\n";

    if (m_total_stack_size > 0) {
        // 从相对于 fp 的偏移处恢复寄存器
        // 恢复所有用到的 callee-saved 寄存器
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

// loadOperand 和 storeOperand 无需改动, 它们依赖 prepare 生成的 m_locations
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
            // 在栈上, 执行 lw
            ss << "  lw " << destReg << ", " << loc << "\n";
        }
        else {
            // 在寄存器里, 执行 mv
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
            // 目标在栈上, 执行 sw
            ss << "  sw " << srcReg << ", " << loc << "\n";
        }
        else {
            // 目标在寄存器里, 执行 mv
            if (srcReg != loc) {
                ss << "  mv " << loc << ", " << srcReg << "\n";
            }
        }
    }
    return ss.str();
}