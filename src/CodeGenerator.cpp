#include "CodeGenerator.hpp"
#include <iostream>
#include <stdexcept>
#include <algorithm>

// ������������ Operand ת��Ϊ�ַ�����ʽ�ı�ʶ��
std::string operand_to_key(const Operand& op) {
    switch (op.kind) {
    case Operand::VAR:
        return op.name;
    case Operand::TEMP:
        return "t" + std::to_string(op.id);
    default:
        return "";
    }
}

// ������������ջƫ����ת��Ϊ�������е� "(fp)" ��ʽ
std::string offset_to_string(int offset) {
    return std::to_string(offset) + "(fp)";
}

// �����ع��ĺ��ģ�ʵ�����޳�ͻ��ͳһջ֡����
void CodeGenerator::generate_function(const FunctionIR& func) {
    // --- ���� 1: Ԥɨ�貢����ջ֡���� ---
    std::map<std::string, int> stack_offsets;
    int current_offset = 0;

    // *** ���ĸĶ���ra �� fp Ҳ��Ϊջ��һ���ֽ���ͳһ���� ***
    // ����Ϊ��Ҫ����ļĴ��� ra �� fp ���䡰��ʥ�����ַ�����ר���ռ�
    current_offset -= 4;
    const int ra_offset = current_offset; // ra ����Զ������ fp-4
    current_offset -= 4;
    const int old_fp_offset = current_offset; // ��fp ����Զ������ fp-8

    // ���ڣ��ſ�ʼΪ�������б�������ռ䣬current_offset �� -8 ��ʼ
    auto allocate_if_needed = [&](const Operand& op) {
        std::string key = operand_to_key(op);
        if (!key.empty() && stack_offsets.find(key) == stack_offsets.end()) {
            current_offset -= 4;
            stack_offsets[key] = current_offset;
        }
    };

    for (const auto& param : func.params) {
        Operand param_op = { Operand::VAR, param.name };
        allocate_if_needed(param_op);
    }

    for (const auto& bb : func.blocks) {
        for (const auto& instr : bb.instructions) {
            allocate_if_needed(instr.result);
            allocate_if_needed(instr.arg1);
            allocate_if_needed(instr.arg2);
        }
    }

    int total_stack_size = -current_offset;
    if (total_stack_size % 16 != 0) {
        total_stack_size += 16 - (total_stack_size % 16);
    }

    // --- ���� 2: ����ȫ�µġ��޳�ͻ�ĺ������� ---
    m_output << func.name << ":\n";
    if (total_stack_size > 0) {
        m_output << "  addi sp, sp, -" << total_stack_size << "\n";
        // 2. ʹ�á������ sp����ƫ�ƣ����� ra �� �� fp
        //    ra_offset ��-4, old_fp_offset ��-8��������� sp �ĵ�ַ�� total_stack_size + offset��
        //    ��Ϊ offset_to_string() ǿ��ʹ�� (fp)�������������ֶ�ƴ���ַ�����
        m_output << "  sw ra, " << (total_stack_size - 4) << "(sp)\n";
        m_output << "  sw fp, " << (total_stack_size - 8) << "(sp)\n";
    }

    // 3. �ڱ�����������Ϣ�󣬲Ÿ��� fp
    if (total_stack_size > 0) {
        m_output << "  addi fp, sp, " << total_stack_size << "\n";
    }


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
            generate_instruction(instr, stack_offsets, total_stack_size);
        }
    }
}

// generate_instruction ��������ȫ�µ� offsets ��
void CodeGenerator::generate_instruction(const Instruction& instr, const std::map<std::string, int>& offsets, int total_stack_size) {
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
    case Instruction::ADD: case Instruction::SUB: case Instruction::MUL: case Instruction::DIV: case Instruction::MOD: {
        load_op(instr.arg1, "t0"); load_op(instr.arg2, "t1");
        std::string op_str = "add";
        if (instr.opcode == Instruction::SUB) op_str = "sub";
        if (instr.opcode == Instruction::MUL) op_str = "mul";
        if (instr.opcode == Instruction::DIV) op_str = "div";
        if (instr.opcode == Instruction::MOD) op_str = "rem";
        m_output << "  " << op_str << " t2, t0, t1\n";
        store_op(instr.result, "t2");
        break;
    }
    case Instruction::LE: {
        load_op(instr.arg1, "t0"); load_op(instr.arg2, "t1");
        m_output << "  sgt t2, t0, t1\n"; m_output << "  xori t2, t2, 1\n";
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
        // ֱ��ʹ�ô���� total_stack_size���������¼���
        // β���� lw ��Ȼ����ʹ�� (fp) ���Ѱַ����Ϊ fp ���������Ѿ�������ȷ
        if (total_stack_size > 0) {
            const int ra_off = -4;
            const int old_fp_off = -8;
            m_output << "  lw ra, " << offset_to_string(ra_off) << "\n";
            m_output << "  lw fp, " << offset_to_string(old_fp_off) << "\n";
            m_output << "  addi sp, sp, " << total_stack_size << "\n";
        }
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
        if (instr.result.kind != Operand::NONE) { store_op(instr.result, "a0"); }
        break;
    case Instruction::LABEL:
        m_output << instr.arg1.name << ":\n";
        break;
    default:
        m_output << "  # Unhandled OpCode: " << instr.opcode << "\n";
        break;
    }
}

// ����ں������ֲ���
std::string CodeGenerator::generate(const ModuleIR& module) {
    m_output.str(""); m_output.clear();
    m_output << ".text\n"; m_output << ".globl main\n\n";
    m_param_idx = 0;

    const FunctionIR* main_func = nullptr;
    for (const auto& func : module.functions) {
        if (func.name == "main") { main_func = &func; break; }
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