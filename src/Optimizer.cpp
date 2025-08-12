#include "Optimizer.hpp"
#include "IRGenerator.hpp" 
#include <unordered_set>
#include <string>
#include <vector>

// --- �������� ---

// ���һ���������Ƿ��Ǳ�������ʱ����
static bool is_variable_or_temp(const Operand& op) {
    return op.kind == Operand::VAR || op.kind == Operand::TEMP;
}

// Ϊ��������ʱ��������һ��Ψһ���ַ�����ʶ���������ڼ�����׷��
static std::string get_operand_id(const Operand& op) {
    if (op.kind == Operand::VAR) {
        return op.name;
    }
    if (op.kind == Operand::TEMP) {
        return "t" + std::to_string(op.id);
    }
    return ""; // �������Ͳ�����û��ID
}


void Optimizer::optimize(ModuleIR& module) {
    // --- �׶�һ: �����۵� ---
    // (��֮ǰ�߼���ͬ�����Ƴ��˲�����ֵ� AND �� OR ��֧)
    for (auto& func : module.functions) {
        for (auto& block : func.blocks) {
            for (auto& instr : block.instructions) {
                if (instr.arg1.kind == Operand::CONST && instr.arg2.kind == Operand::CONST) {
                    int val1 = instr.arg1.value;
                    int val2 = instr.arg2.value;
                    int result = 0;
                    bool optimized = true;

                    switch (instr.opcode) {
                        case Instruction::ADD: result = val1 + val2; break;
                        case Instruction::SUB: result = val1 - val2; break;
                        case Instruction::MUL: result = val1 * val2; break;
                        case Instruction::DIV:
                            if (val2 == 0) { optimized = false; break; }
                            result = val1 / val2;
                            break;
                        case Instruction::MOD:
                             if (val2 == 0) { optimized = false; break; }
                            result = val1 % val2;
                            break;
                        case Instruction::EQ:  result = (val1 == val2); break;
                        case Instruction::NEQ: result = (val1 != val2); break;
                        case Instruction::LT:  result = (val1 < val2);  break;
                        case Instruction::GT:  result = (val1 > val2);  break;
                        case Instruction::LE:  result = (val1 <= val2); break;
                        case Instruction::GE:  result = (val1 >= val2); break;
                        default:
                            optimized = false;
                            break;
                    }

                    if (optimized) {
                        instr.opcode = Instruction::ASSIGN;
                        instr.arg1.kind = Operand::CONST;
                        instr.arg1.value = result;
                        instr.arg2.kind = Operand::NONE; 
                    }
                } else if (instr.arg1.kind == Operand::CONST && instr.arg2.kind == Operand::NONE) {
                    if (instr.opcode == Instruction::NOT) {
                        instr.opcode = Instruction::ASSIGN;
                        instr.arg1.value = !instr.arg1.value;
                    }
                }
            }
        }
    }

    // --- �׶ζ�: ���������� (��������) ---
    bool changed_in_pass = true;
    // ֻҪ��һ���д��뱻�������ͼ���ѭ����ֱ��û�д��������Ϊֹ
    while (changed_in_pass) {
        changed_in_pass = false;

        for (auto& func : module.functions) {
            // 1. �ռ����б�"ʹ��"�ı�������ʱ����
            std::unordered_set<std::string> used_operands;
            for (auto& block : func.blocks) {
                for (auto& instr : block.instructions) {
                    // �������� arg1 �� arg2 �г��֣���Ϊ"ʹ��"
                    if (is_variable_or_temp(instr.arg1)) {
                        used_operands.insert(get_operand_id(instr.arg1));
                    }
                    if (is_variable_or_temp(instr.arg2)) {
                        used_operands.insert(get_operand_id(instr.arg2));
                    }
                    
                    // ����ʹ�������
                    // - `RET val`: val ��ʹ��
                    // - `JUMP_IF cond`: cond ��ʹ��
                    // - `PARAM p`: p ��ʹ��
                    if (instr.opcode == Instruction::RET || 
                        instr.opcode == Instruction::JUMP_IF_ZERO ||
                        instr.opcode == Instruction::JUMP_IF_NZERO ||
                        instr.opcode == Instruction::PARAM) 
                    {
                        if (is_variable_or_temp(instr.arg1)) {
                            used_operands.insert(get_operand_id(instr.arg1));
                        }
                    }
                }
            }

            // 2. �������Ƴ������δ��ʹ�õ�ָ��
            for (auto& block : func.blocks) {
                std::vector<Instruction> new_instructions;
                new_instructions.reserve(block.instructions.size());

                for (const auto& instr : block.instructions) {
                    bool is_dead = false;
                    // ���ָ���Ƿ�����һ����δ��ʹ�õĽ��
                    if (is_variable_or_temp(instr.result)) {
                        // �����"ʹ��"�������Ҳ����������������п�����������
                        if (used_operands.find(get_operand_id(instr.result)) == used_operands.end()) {
                            // �ٴ�ȷ�ϸ�ָ��û�и����� (Side Effect)
                            switch (instr.opcode) {
                                // ��Щָ���Ǵ����㣬���û����ʹ����������������
                                case Instruction::ADD:
                                case Instruction::SUB:
                                case Instruction::MUL:
                                case Instruction::DIV:
                                case Instruction::MOD:
                                case Instruction::NOT:
                                case Instruction::EQ:
                                case Instruction::NEQ:
                                case Instruction::LT:
                                case Instruction::GT:
                                case Instruction::LE:
                                case Instruction::GE:
                                case Instruction::ASSIGN:
                                    is_dead = true;
                                    break;
                                
                                // CALL ָ���и����ã���ʹ����ֵ���ã�Ҳ����ɾ��
                                case Instruction::CALL:
                                default:
                                    is_dead = false;
                                    break;
                            }
                        }
                    }

                    if (is_dead) {
                        changed_in_pass = true; // ��Ǳ����д��뱻ɾ��
                    } else {
                        new_instructions.push_back(instr);
                    }
                }
                // ���µ�ָ���б��滻�ɵ�
                block.instructions = std::move(new_instructions);
            }
        }
    }
}
