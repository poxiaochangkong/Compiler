#include "CodeGenerator.hpp"
#include <stdexcept>
#include <iostream>
#include <map>
#include <algorithm>

// 辅助函数：将 Operand 转换为字符串形式的标识符，用于在 map 中作为 key
// 注意：这个函数现在主要用于为 stack_offsets 映射表提供唯一的键









//void CodeGenerator::generate_block(const BasicBlock& block) {
//    m_output << block.label << ":\n";
//    for (const auto& instr : block.instructions) {
//        generate_instruction(instr);
//    }
//}

void CodeGenerator::alloc_vars(const FunctionIR& func) {
    m_var_offsets.clear();
    int current_offset = 0;

    // 首先为参数分配空间
    for (const auto& param : func.params) {
        current_offset -= 4; // 栈向下增长
        m_var_offsets[param.name] = current_offset;
    }

    // 然后为所有具名变量和临时变量分配空间
    for (const auto& block : func.blocks) {
        for (const auto& instr : block.instructions) {
            auto check_and_alloc = [&](const Operand& op) {
                if (op.kind == Operand::VAR || op.kind == Operand::TEMP) {
                    std::string name = (op.kind == Operand::VAR) ? op.name : "t" + std::to_string(op.id);
                    if (m_var_offsets.find(name) == m_var_offsets.end()) {
                        current_offset -= 4;
                        m_var_offsets[name] = current_offset;
                    }
                }
            };
            check_and_alloc(instr.result);
            check_and_alloc(instr.arg1);
            check_and_alloc(instr.arg2);
        }
    }

    // RISC-V 要求栈大小是 16 字节对齐的
    int size = -current_offset;
    if (size % 16 != 0) {
        size = (size / 16 + 1) * 16;
    }
    m_current_stack_size = size;
}

int CodeGenerator::get_var_offset(const Operand& op) {
    std::string name;
    if (op.kind == Operand::VAR) {
        name = op.name;
    }
    else if (op.kind == Operand::TEMP) {
        name = "t" + std::to_string(op.id);
    }
    else {
        // 对于非变量操作数，不应该查询偏移量
        throw std::runtime_error("Cannot get offset for non-variable operand");
    }

    if (m_var_offsets.find(name) != m_var_offsets.end()) {
        return m_var_offsets.at(name);
    }
    throw std::runtime_error("Variable not found in offset map: " + name);
}

void CodeGenerator::emit_prologue() {
    // fp/s0 是 callee-saved register
    // sp: stack pointer, fp: frame pointer, ra: return address
    m_output << "  addi sp, sp, -" << m_current_stack_size << "\n";
    m_output << "  sw ra, " << m_current_stack_size - 4 << "(sp)\n";
    m_output << "  sw fp, " << m_current_stack_size - 8 << "(sp)\n";
    m_output << "  addi fp, sp, " << m_current_stack_size << "\n";
}

void CodeGenerator::emit_epilogue() {
    m_output << "  lw ra, " << m_current_stack_size - 4 << "(sp)\n";
    m_output << "  lw fp, " << m_current_stack_size - 8 << "(sp)\n";
    m_output << "  addi sp, sp, " << m_current_stack_size << "\n";
    m_output << "  ret\n";
}

std::string CodeGenerator::ensure_in_register(const Operand& op, int reg_idx) {
    std::string reg = "t" + std::to_string(reg_idx);
    if (op.kind == Operand::CONST) {
        m_output << "  li " << reg << ", " << op.value << "\n"; // li: load immediate
    }
    else {
        int offset = get_var_offset(op);
        m_output << "  lw " << reg << ", " << offset << "(fp)\n"; // lw: load word
    }
    return reg;
}

// 辅助函数：将 Operand 转换为字符串形式的标识符，用于在 map 中作为 key
std::string operand_to_key(const Operand& op) {
    switch (op.kind) {
    case Operand::VAR:
        return op.name; // 变量，如 "n"
    case Operand::TEMP:
        return "t" + std::to_string(op.id); // 临时变量，如 "t0"
    default:
        return ""; // 常量和标签没有栈位置
    }
}

// 辅助函数：将栈偏移量转换为汇编代码中的 "(fp)" 格式
std::string offset_to_string(int offset) {
    return std::to_string(offset) + "(fp)";
}

// 这是重构的核心，实现了统一的栈帧管理
void CodeGenerator::generate_function(const FunctionIR& func) {
    // --- 步骤 1: 预扫描并计算栈帧布局 ---
    std::map<std::string, int> stack_offsets;
    int current_offset = 0; // 我们将从 fp 向下（负方向）分配

    auto allocate_if_needed = [&](const Operand& op) {
        std::string key = operand_to_key(op);
        if (!key.empty() && stack_offsets.find(key) == stack_offsets.end()) {
            current_offset -= 4; // 这是一个新的变量/临时值，为它分配4字节空间
            stack_offsets[key] = current_offset;
        }
    };

    // 首先为所有函数参数分配栈空间
    for (const auto& param : func.params) {
        Operand param_op;
        param_op.kind = Operand::VAR;
        param_op.name = param.name;
        allocate_if_needed(param_op);
    }

    // 接着为函数体中所有用到的变量和临时值分配栈空间
    for (const auto& bb : func.blocks) {
        for (const auto& instr : bb.instructions) {
            allocate_if_needed(instr.result);
            allocate_if_needed(instr.arg1);
            allocate_if_needed(instr.arg2);
        }
    }

    // 栈帧总大小 = 所有变量占用的空间 + 保存ra/fp的8字节
    int total_stack_size = -current_offset;
    total_stack_size += 8;
    // 确保栈帧大小是16字节对齐的，这是RISC-V ABI的要求
    if (total_stack_size % 16 != 0) {
        total_stack_size += 16 - (total_stack_size % 16);
    }

    // --- 步骤 2: 生成函数序言 (Prologue) ---
    m_output << func.name << ":\n";
    m_output << "  addi sp, sp, -" << total_stack_size << "\n";
    // ra 和 fp 总是保存在栈帧的最高地址处，与所有局部变量完全分开
    m_output << "  sw ra, " << total_stack_size - 4 << "(sp)\n";
    m_output << "  sw fp, " << total_stack_size - 8 << "(sp)\n";
    m_output << "  addi fp, sp, " << total_stack_size << "\n";

    // 将传入的参数寄存器（a0, a1, ...）的值，保存到它们在栈上的指定位置
    for (size_t i = 0; i < func.params.size(); ++i) {
        std::string param_name = func.params[i].name;
        if (stack_offsets.count(param_name)) {
            m_output << "  sw a" << i << ", " << offset_to_string(stack_offsets[param_name]) << "\n";
        }
    }

    // --- 步骤 3: 生成函数体 ---
    for (const auto& bb : func.blocks) {
        if (!bb.label.empty()) {
            m_output << bb.label << ":\n";
        }
        for (const auto& instr : bb.instructions) {
            generate_instruction(instr, stack_offsets);
        }
    }
}

// 这个函数现在完全依赖于 offsets 表来访问内存，保证了正确性
void CodeGenerator::generate_instruction(const Instruction& instr, const std::map<std::string, int>& offsets) {
    auto get_offset_str = [&](const Operand& op) {
        std::string key = operand_to_key(op);
        return offsets.count(key) ? offset_to_string(offsets.at(key)) : "??_UNKNOWN_??";
    };

    auto load_op = [&](const Operand& op, const std::string& reg) {
        if (op.kind == Operand::CONST) {
            m_output << "  li " << reg << ", " << op.value << "\n";
        }
        else {
            m_output << "  lw " << reg << ", " << get_offset_str(op) << "\n";
        }
    };

    auto store_op = [&](const Operand& op, const std::string& reg) {
        m_output << "  sw " << reg << ", " << get_offset_str(op) << "\n";
    };

    switch (instr.opcode) {
    case Instruction::ADD:
    case Instruction::SUB:
    case Instruction::MUL:
    case Instruction::DIV:
    case Instruction::MOD: {
        load_op(instr.arg1, "t0");
        load_op(instr.arg2, "t1");
        std::string op_str;
        if (instr.opcode == Instruction::ADD) op_str = "add";
        else if (instr.opcode == Instruction::SUB) op_str = "sub";
        else if (instr.opcode == Instruction::MUL) op_str = "mul";
        else if (instr.opcode == Instruction::DIV) op_str = "div";
        else op_str = "rem";
        m_output << "  " << op_str << " t2, t0, t1\n";
        store_op(instr.result, "t2");
        break;
    }

    case Instruction::LE: {
        load_op(instr.arg1, "t0");
        load_op(instr.arg2, "t1");
        m_output << "  sgt t2, t0, t1\n";
        m_output << "  xori t2, t2, 1\n";
        store_op(instr.result, "t2");
        break;
    }

    case Instruction::JUMP_IF_ZERO:
        load_op(instr.arg1, "t0");
        m_output << "  beqz t0, " << instr.arg2.name << "\n";
        break;

    case Instruction::JUMP:
        m_output << "  j " << instr.arg1.name << "\n";
        break;

    case Instruction::ASSIGN:
        load_op(instr.arg1, "t0");
        store_op(instr.result, "t0");
        break;

    case Instruction::RET: {
        if (instr.arg1.kind != Operand::NONE) {
            load_op(instr.arg1, "a0");
        }
        // 函数尾声
        int total_stack_size = 0;
        if (!offsets.empty()) {
            int min_offset = 0;
            for (std::map<std::string, int>::const_iterator it = offsets.begin(); it != offsets.end(); ++it) {
                min_offset = std::min(min_offset, it->second);
            }
            total_stack_size = -min_offset + 8;
            if (total_stack_size % 16 != 0) { total_stack_size += 16 - (total_stack_size % 16); }
        }
        else { total_stack_size = 16; }
        m_output << "  lw ra, " << total_stack_size - 4 << "(sp)\n";
        m_output << "  lw fp, " << total_stack_size - 8 << "(sp)\n";
        m_output << "  addi sp, sp, " << total_stack_size << "\n";
        m_output << "  ret\n";
        break;
    }

    case Instruction::PARAM:
        load_op(instr.arg1, "a" + std::to_string(m_param_idx));
        m_param_idx++;
        break;

    case Instruction::CALL:
        m_param_idx = 0;
        m_output << "  call " << instr.arg1.name << "\n";
        if (instr.result.kind != Operand::NONE) {
            store_op(instr.result, "a0");
        }
        break;

    case Instruction::LABEL:
        m_output << instr.arg1.name << ":\n";
        break;

    default:
        m_output << "  # Unhandled OpCode: " << instr.opcode << "\n";
        break;
    }
}

// 主入口函数，现在包含了 m_param_idx 的初始化
std::string CodeGenerator::generate(const ModuleIR& module) {
    m_output.str("");
    m_output.clear();

    m_output << ".text\n";
    m_output << ".globl main\n\n";

    m_param_idx = 0;

    const FunctionIR* main_func = nullptr;
    for (const auto& func : module.functions) {
        if (func.name == "main") {
            main_func = &func;
            break;
        }
    }

    if (main_func) {
        generate_function(*main_func);
        m_output << "\n";
    }
    else {
        throw std::runtime_error("Error: 'main' function not found in module.");
    }

    for (const auto& func : module.functions) {
        if (func.name != "main") {
            generate_function(func);
            m_output << "\n";
        }
    }

    return m_output.str();
}
