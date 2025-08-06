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

void SpillEverythingAllocator::prepare(const FunctionIR& func) {
    m_func_name = func.name;
    m_stack_offsets.clear();
    int current_offset = 0;

    // 为 ra 和 fp 分配固定位置
    current_offset -= 4;
    const int ra_offset = current_offset;
    m_stack_offsets["<ra>"] = ra_offset; // 使用特殊键名
    current_offset -= 4;
    const int old_fp_offset = current_offset;
    m_stack_offsets["<old_fp>"] = old_fp_offset;

    // 为所有变量和临时结果分配栈空间
    auto allocate_if_needed = [&](const Operand& op) {
        std::string key = operandToKey(op);
        if (!key.empty() && m_stack_offsets.find(key) == m_stack_offsets.end()) {
            current_offset -= 4;
            m_stack_offsets[key] = current_offset;
        }
    };

    for (const auto& param : func.params) allocate_if_needed({ Operand::VAR, param.name });
    for (const auto& bb : func.blocks) {
        for (const auto& instr : bb.instructions) {
            allocate_if_needed(instr.result);
            allocate_if_needed(instr.arg1);
            allocate_if_needed(instr.arg2);
        }
    }

    m_total_stack_size = -current_offset;
    // 16字节对齐
    if (m_total_stack_size % 16 != 0) {
        m_total_stack_size += 16 - (m_total_stack_size % 16);
    }

    std::stringstream ss;
    for (size_t i = 0; i < func.params.size(); ++i) {
        std::string key = operandToKey({ Operand::VAR, func.params[i].name });
        if (m_stack_offsets.count(key)) {
            ss << "  sw a" << i << ", " << offsetToString(m_stack_offsets[key]) << "\n";
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
    // 添加参数保存指令
    if (!m_param_init_code.empty()) {
        ss << m_param_init_code;
    }

    return ss.str();
}

std::string SpillEverythingAllocator::getEpilogue() {
    std::stringstream ss;
    if (m_total_stack_size > 0) {
        ss << "  lw ra, " << offsetToString(m_stack_offsets["<ra>"]) << "\n";
        ss << "  lw fp, " << offsetToString(m_stack_offsets["<old_fp>"]) << "\n";
        ss << "  addi sp, sp, " << m_total_stack_size << "\n";
    }
    ss << "  ret\n";
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