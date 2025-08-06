#include "CodeGenerator.hpp"
#include <stdexcept>
#include <algorithm>

std::string CodeGenerator::generate(const ModuleIR& module) {
    // .text 段存放代码
    m_output << ".text\n";
    // .globl main 使得 main 函数可以被链接器找到
    m_output << ".globl main\n";

    for (const auto& func : module.functions) {
        generate_function(func);
    }
    return m_output.str();
}

void CodeGenerator::generate_function(const FunctionIR& func) {
    m_current_func = &func;

    // 1. 分析函数，为所有局部变量和临时变量分配栈空间
    alloc_vars(func);

    // 2. 函数标签
    m_output << "\n" << func.name << ":\n";

    // 3. 函数序言 (Prologue)
    emit_prologue();

    // 4. 将传入的参数（在前8个参数寄存器 a0-a7 中）保存到栈上
    for (size_t i = 0; i < func.params.size(); ++i) {
        if (i < 8) {
            int offset = get_var_offset({ Operand::VAR, func.params[i].name });
            m_output << "  sw a" << i << ", " << offset << "(fp)\n";
        }
    }

    // 5. 遍历基本块，生成指令
    for (const auto& block : func.blocks) {
        generate_block(block);
    }
}

void CodeGenerator::generate_block(const BasicBlock& block) {
    m_output << block.label << ":\n";
    for (const auto& instr : block.instructions) {
        generate_instruction(instr);
    }
}

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

void CodeGenerator::generate_instruction(const Instruction& instr) {
    switch (instr.opcode) {
    case Instruction::ADD:
    case Instruction::SUB:
    case Instruction::MUL:
    case Instruction::DIV:
    case Instruction::MOD:
    case Instruction::LT:
    case Instruction::GT: {
        // 【修正】在这里声明并初始化 op_str
        std::string op_str;
        switch (instr.opcode) {
        case Instruction::ADD: op_str = "add"; break;
        case Instruction::SUB: op_str = "sub"; break;
        case Instruction::MUL: op_str = "mul"; break;
        case Instruction::DIV: op_str = "div"; break;
        case Instruction::MOD: op_str = "rem"; break;
        case Instruction::LT:  op_str = "slt"; break;
        case Instruction::GT:  op_str = "sgt"; break;
        default: break; // Should not happen
        }
        std::string reg1 = ensure_in_register(instr.arg1, 0);
        std::string reg2 = ensure_in_register(instr.arg2, 1);
        m_output << "  " << op_str << " t2, " << reg1 << ", " << reg2 << "\n";
        int dest_offset = get_var_offset(instr.result);
        m_output << "  sw t2, " << dest_offset << "(fp)\n";
        break;
    }
    case Instruction::EQ: { // t2 = (t0 == t1) -> t2 = !(t0 - t1)
        std::string reg1 = ensure_in_register(instr.arg1, 0);
        std::string reg2 = ensure_in_register(instr.arg2, 1);
        m_output << "  sub t2, " << reg1 << ", " << reg2 << "\n";
        m_output << "  seqz t2, t2\n"; // Set if equal to zero
        int dest_offset = get_var_offset(instr.result);
        m_output << "  sw t2, " << dest_offset << "(fp)\n";
        break;
    }
    case Instruction::NEQ: { // t2 = (t0 != t1)
        std::string reg1 = ensure_in_register(instr.arg1, 0);
        std::string reg2 = ensure_in_register(instr.arg2, 1);
        m_output << "  sub t2, " << reg1 << ", " << reg2 << "\n";
        m_output << "  snez t2, t2\n"; // Set if not equal to zero
        int dest_offset = get_var_offset(instr.result);
        m_output << "  sw t2, " << dest_offset << "(fp)\n";
        break;
    }
    case Instruction::LE: { // t0 <= t1  -> !(t1 < t0)
        std::string reg1 = ensure_in_register(instr.arg1, 0);
        std::string reg2 = ensure_in_register(instr.arg2, 1);
        m_output << "  sgt t2, " << reg1 << ", " << reg2 << "\n"; // t2 = t0 > t1
        m_output << "  xori t2, t2, 1\n"; // t2 = !t2
        int dest_offset = get_var_offset(instr.result);
        m_output << "  sw t2, " << dest_offset << "(fp)\n";
        break;
    }
    case Instruction::GE: { // t0 >= t1 -> !(t0 < t1)
        std::string reg1 = ensure_in_register(instr.arg1, 0);
        std::string reg2 = ensure_in_register(instr.arg2, 1);
        m_output << "  slt t2, " << reg1 << ", " << reg2 << "\n"; // t2 = t0 < t1
        m_output << "  xori t2, t2, 1\n"; // t2 = !t2
        int dest_offset = get_var_offset(instr.result);
        m_output << "  sw t2, " << dest_offset << "(fp)\n";
        break;
    }
    case Instruction::ASSIGN: {
        std::string reg1 = ensure_in_register(instr.arg1, 0);
        int dest_offset = get_var_offset(instr.result);
        m_output << "  sw " << reg1 << ", " << dest_offset << "(fp)\n";
        break;
    }
    case Instruction::JUMP: {
        m_output << "  j " << instr.arg1.name << "\n";
        break;
    }
    case Instruction::JUMP_IF_ZERO: {
        std::string reg1 = ensure_in_register(instr.arg1, 0);
        m_output << "  beqz " << reg1 << ", " << instr.arg2.name << "\n";
        break;
    }
    case Instruction::JUMP_IF_NZERO: {
        std::string reg1 = ensure_in_register(instr.arg1, 0);
        m_output << "  bnez " << reg1 << ", " << instr.arg2.name << "\n";
        break;
    }
    case Instruction::PARAM: {
        if (m_param_count < 8) {
            std::string reg = ensure_in_register(instr.arg1, 0);
            m_output << "  mv a" << m_param_count << ", " << reg << "\n";
        }
        m_param_count++;
        break;
    }
    case Instruction::CALL: {
        m_param_count = 0; // 【修正】在发起调用前重置参数计数器
        m_output << "  call " << instr.arg1.name << "\n";
        int dest_offset = get_var_offset(instr.result);
        m_output << "  sw a0, " << dest_offset << "(fp)\n";
        break;
    }
    case Instruction::RET: {
        if (instr.arg1.kind != Operand::NONE) {
            ensure_in_register(instr.arg1, 0);
            m_output << "  mv a0, t0\n";
        }
        emit_epilogue();
        break;
    }
    default:
        break;
    }
}
