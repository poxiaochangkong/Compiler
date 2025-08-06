#include "CodeGenerator.hpp"
#include <stdexcept>
#include <iostream>
#include <map>
#include <algorithm>

// ������������ Operand ת��Ϊ�ַ�����ʽ�ı�ʶ���������� map ����Ϊ key
// ע�⣺�������������Ҫ����Ϊ stack_offsets ӳ����ṩΨһ�ļ�









//void CodeGenerator::generate_block(const BasicBlock& block) {
//    m_output << block.label << ":\n";
//    for (const auto& instr : block.instructions) {
//        generate_instruction(instr);
//    }
//}

void CodeGenerator::alloc_vars(const FunctionIR& func) {
    m_var_offsets.clear();
    int current_offset = 0;

    // ����Ϊ��������ռ�
    for (const auto& param : func.params) {
        current_offset -= 4; // ջ��������
        m_var_offsets[param.name] = current_offset;
    }

    // Ȼ��Ϊ���о�����������ʱ��������ռ�
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

    // RISC-V Ҫ��ջ��С�� 16 �ֽڶ����
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
        // ���ڷǱ�������������Ӧ�ò�ѯƫ����
        throw std::runtime_error("Cannot get offset for non-variable operand");
    }

    if (m_var_offsets.find(name) != m_var_offsets.end()) {
        return m_var_offsets.at(name);
    }
    throw std::runtime_error("Variable not found in offset map: " + name);
}

void CodeGenerator::emit_prologue() {
    // fp/s0 �� callee-saved register
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

// ������������ Operand ת��Ϊ�ַ�����ʽ�ı�ʶ���������� map ����Ϊ key
std::string operand_to_key(const Operand& op) {
    switch (op.kind) {
    case Operand::VAR:
        return op.name; // �������� "n"
    case Operand::TEMP:
        return "t" + std::to_string(op.id); // ��ʱ�������� "t0"
    default:
        return ""; // �����ͱ�ǩû��ջλ��
    }
}

// ������������ջƫ����ת��Ϊ�������е� "(fp)" ��ʽ
std::string offset_to_string(int offset) {
    return std::to_string(offset) + "(fp)";
}

// �����ع��ĺ��ģ�ʵ����ͳһ��ջ֡����
void CodeGenerator::generate_function(const FunctionIR& func) {
    // --- ���� 1: Ԥɨ�貢����ջ֡���� ---
    std::map<std::string, int> stack_offsets;
    int current_offset = 0; // ���ǽ��� fp ���£������򣩷���

    auto allocate_if_needed = [&](const Operand& op) {
        std::string key = operand_to_key(op);
        if (!key.empty() && stack_offsets.find(key) == stack_offsets.end()) {
            current_offset -= 4; // ����һ���µı���/��ʱֵ��Ϊ������4�ֽڿռ�
            stack_offsets[key] = current_offset;
        }
    };

    // ����Ϊ���к�����������ջ�ռ�
    for (const auto& param : func.params) {
        Operand param_op;
        param_op.kind = Operand::VAR;
        param_op.name = param.name;
        allocate_if_needed(param_op);
    }

    // ����Ϊ�������������õ��ı�������ʱֵ����ջ�ռ�
    for (const auto& bb : func.blocks) {
        for (const auto& instr : bb.instructions) {
            allocate_if_needed(instr.result);
            allocate_if_needed(instr.arg1);
            allocate_if_needed(instr.arg2);
        }
    }

    // ջ֡�ܴ�С = ���б���ռ�õĿռ� + ����ra/fp��8�ֽ�
    int total_stack_size = -current_offset;
    total_stack_size += 8;
    // ȷ��ջ֡��С��16�ֽڶ���ģ�����RISC-V ABI��Ҫ��
    if (total_stack_size % 16 != 0) {
        total_stack_size += 16 - (total_stack_size % 16);
    }

    // --- ���� 2: ���ɺ������� (Prologue) ---
    m_output << func.name << ":\n";
    m_output << "  addi sp, sp, -" << total_stack_size << "\n";
    // ra �� fp ���Ǳ�����ջ֡����ߵ�ַ���������оֲ�������ȫ�ֿ�
    m_output << "  sw ra, " << total_stack_size - 4 << "(sp)\n";
    m_output << "  sw fp, " << total_stack_size - 8 << "(sp)\n";
    m_output << "  addi fp, sp, " << total_stack_size << "\n";

    // ������Ĳ����Ĵ�����a0, a1, ...����ֵ�����浽������ջ�ϵ�ָ��λ��
    for (size_t i = 0; i < func.params.size(); ++i) {
        std::string param_name = func.params[i].name;
        if (stack_offsets.count(param_name)) {
            m_output << "  sw a" << i << ", " << offset_to_string(stack_offsets[param_name]) << "\n";
        }
    }

    // --- ���� 3: ���ɺ����� ---
    for (const auto& bb : func.blocks) {
        if (!bb.label.empty()) {
            m_output << bb.label << ":\n";
        }
        for (const auto& instr : bb.instructions) {
            generate_instruction(instr, stack_offsets);
        }
    }
}

// �������������ȫ������ offsets ���������ڴ棬��֤����ȷ��
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
        // ����β��
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

// ����ں��������ڰ����� m_param_idx �ĳ�ʼ��
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
