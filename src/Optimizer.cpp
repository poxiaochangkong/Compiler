#include "Optimizer.hpp"
#include "IRGenerator.hpp" 
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>

// --- �������� (���ֲ���) ---

static bool is_variable_or_temp(const Operand& op) {
    return op.kind == Operand::VAR || op.kind == Operand::TEMP;
}

static std::string get_operand_id(const Operand& op) {
    if (op.kind == Operand::VAR) return op.name;
    if (op.kind == Operand::TEMP) return "t" + std::to_string(op.id);
    return "";
}

// --- ��Э������ ---

void Optimizer::optimize(ModuleIR& module) {
    // ���� 1: �����۵�����ͨ����Ϊ��һ������ִ��һ�μ��ɡ�
    run_constant_folding(module);

    // ���� 2: �������и�д������������������ֱ�����벻�ٱ仯��
    bool changed_in_cycle = true;
    while (changed_in_cycle) {
        // �������������Ż�
        bool cp_changed = run_copy_propagation(module);
        bool dce_changed = run_dead_code_elimination(module);

        // ��������κ�һ���Ż��޸��˴��룬����Ҫ�ٽ���һ��ѭ��
        changed_in_cycle = cp_changed || dce_changed;
    }
}


// --- �Ż��׶�һ: �����۵� ---

void Optimizer::run_constant_folding(ModuleIR& module) {
    for (auto& func : module.functions) {
        for (auto& block : func.blocks) {
            for (auto& instr : block.instructions) {
                // �۵�˫������ָ��
                if (instr.arg1.kind == Operand::CONST && instr.arg2.kind == Operand::CONST) {
                    int val1 = instr.arg1.value;
                    int val2 = instr.arg2.value;
                    int result = 0;
                    bool optimized = true;
                    switch (instr.opcode) {
                    case Instruction::ADD: result = val1 + val2; break;
                    case Instruction::SUB: result = val1 - val2; break;
                    case Instruction::MUL: result = val1 * val2; break;
                    case Instruction::DIV: if (val2 == 0) { optimized = false; break; } result = val1 / val2; break;
                    case Instruction::MOD: if (val2 == 0) { optimized = false; break; } result = val1 % val2; break;
                    case Instruction::EQ:  result = (val1 == val2); break;
                    case Instruction::NEQ: result = (val1 != val2); break;
                    case Instruction::LT:  result = (val1 < val2);  break;
                    case Instruction::GT:  result = (val1 > val2);  break;
                    case Instruction::LE:  result = (val1 <= val2); break;
                    case Instruction::GE:  result = (val1 >= val2); break;
                    default: optimized = false; break;
                    }
                    if (optimized) {
                        instr.opcode = Instruction::ASSIGN;
                        instr.arg1.kind = Operand::CONST;
                        instr.arg1.value = result;
                        instr.arg2.kind = Operand::NONE;
                    }
                }
                // �۵���������ָ��
                else if (instr.arg1.kind == Operand::CONST && instr.arg2.kind == Operand::NONE) {
                    if (instr.opcode == Instruction::NOT) {
                        instr.opcode = Instruction::ASSIGN;
                        instr.arg1.value = !instr.arg1.value;
                    }
                }
            }
        }
    }
}

// --- �Ż��׶ζ�: ��д���� ---

bool Optimizer::run_copy_propagation(ModuleIR& module) {
    bool changed_this_pass = false;
    for (auto& func : module.functions) {
        std::unordered_map<std::string, Operand> copy_map;
        for (const auto& block : func.blocks) {
            for (const auto& instr : block.instructions) {
                if (instr.opcode == Instruction::ASSIGN && is_variable_or_temp(instr.result) && is_variable_or_temp(instr.arg1)) {
                    copy_map[get_operand_id(instr.result)] = instr.arg1;
                }
            }
        }

        if (copy_map.empty()) continue;

        for (auto& block : func.blocks) {
            for (auto& instr : block.instructions) {
                if (is_variable_or_temp(instr.arg1)) {
                    auto it = copy_map.find(get_operand_id(instr.arg1));
                    if (it != copy_map.end()) {
                        instr.arg1 = it->second;
                        changed_this_pass = true;
                    }
                }
                if (is_variable_or_temp(instr.arg2)) {
                    auto it = copy_map.find(get_operand_id(instr.arg2));
                    if (it != copy_map.end()) {
                        instr.arg2 = it->second;
                        changed_this_pass = true;
                    }
                }
            }
        }
    }
    return changed_this_pass;
}


// --- �Ż��׶���: ���������� ---

bool Optimizer::run_dead_code_elimination(ModuleIR& module) {
    bool changed_at_all = false;
    for (auto& func : module.functions) {
        std::unordered_set<std::string> used_operands;
        for (auto& block : func.blocks) {
            for (auto& instr : block.instructions) {
                if (is_variable_or_temp(instr.arg1)) used_operands.insert(get_operand_id(instr.arg1));
                if (is_variable_or_temp(instr.arg2)) used_operands.insert(get_operand_id(instr.arg2));

                if (instr.opcode == Instruction::RET || instr.opcode == Instruction::JUMP_IF_ZERO ||
                    instr.opcode == Instruction::JUMP_IF_NZERO || instr.opcode == Instruction::PARAM) {
                    if (is_variable_or_temp(instr.arg1)) used_operands.insert(get_operand_id(instr.arg1));
                }
            }
        }

        std::vector<Instruction> new_instructions;
        for (auto& block : func.blocks) {
            new_instructions.clear();
            new_instructions.reserve(block.instructions.size());

            for (const auto& instr : block.instructions) {
                bool is_dead = false;
                if (is_variable_or_temp(instr.result)) {
                    if (used_operands.find(get_operand_id(instr.result)) == used_operands.end()) {
                        switch (instr.opcode) {
                        case Instruction::ADD: case Instruction::SUB: case Instruction::MUL:
                        case Instruction::DIV: case Instruction::MOD: case Instruction::NOT:
                        case Instruction::EQ:  case Instruction::NEQ: case Instruction::LT:
                        case Instruction::GT:  case Instruction::LE:  case Instruction::GE:
                        case Instruction::ASSIGN:
                            is_dead = true;
                            break;
                        default: is_dead = false; break;
                        }
                    }
                }
                if (is_dead) {
                    changed_at_all = true;
                }
                else {
                    new_instructions.push_back(instr);
                }
            }
            block.instructions = std::move(new_instructions);
        }
    }
    return changed_at_all;
}
