#include "Optimizer.hpp"

// ���� Instruction::OpCode, Operand::Kind ��ö��
#include "IRGenerator.hpp" 

void Optimizer::optimize(ModuleIR& module) {
    // ����ģ���е�ÿһ������
    for (auto& func : module.functions) {
        // ���������е�ÿһ��������
        for (auto& block : func.blocks) {
            // �����������е�ÿһ��ָ��
            // ����ʹ������(auto& instr)����������ֱ���޸�ԭʼָ��
            for (auto& instr : block.instructions) {

                // ֻ������ arg1 �� arg2 ��ָ��
                if (instr.arg1.kind == Operand::NONE) {
                    continue;
                }

                // --- �Ż��� 1: ��������ָ�� (���� NOT) ---
                if (instr.arg2.kind == Operand::NONE) {
                    if (instr.arg1.kind == Operand::CONST) {
                        int val = instr.arg1.value;
                        int result = 0;
                        bool optimized = true;

                        if (instr.opcode == Instruction::NOT) {
                            result = !val;
                        }
                        else {
                            optimized = false; // ������������ָ���ݲ�����
                        }

                        if (optimized) {
                            // ԭ���޸�ָ�
                            // 1. �������Ϊ ASSIGN
                            // 2. arg1 ��Ϊ�������ĳ���
                            instr.opcode = Instruction::ASSIGN;
                            instr.arg1.kind = Operand::CONST;
                            instr.arg1.value = result;
                        }
                    }
                    continue; // ������һ��ָ��
                }


                // --- �Ż��� 2: ˫������ָ�� (ADD, SUB, LT, etc.) ---
                // ��������������Ƿ��ǳ���
                if (instr.arg1.kind == Operand::CONST && instr.arg2.kind == Operand::CONST) {
                    int val1 = instr.arg1.value;
                    int val2 = instr.arg2.value;
                    int result = 0;
                    bool optimized = true; // ����Ƿ�ɹ��Ż�

                    switch (instr.opcode) {
                    case Instruction::ADD: result = val1 + val2; break;
                    case Instruction::SUB: result = val1 - val2; break;
                    case Instruction::MUL: result = val1 * val2; break;
                    case Instruction::DIV:
                        if (val2 == 0) { optimized = false; break; } // ���㣬�����Ż�
                        result = val1 / val2;
                        break;
                    case Instruction::MOD:
                        if (val2 == 0) { optimized = false; break; } // ���㣬�����Ż�
                        result = val1 % val2;
                        break;
                    case Instruction::EQ:  result = (val1 == val2); break;
                    case Instruction::NEQ: result = (val1 != val2); break;
                    case Instruction::LT:  result = (val1 < val2);  break;
                    case Instruction::GT:  result = (val1 > val2);  break;
                    case Instruction::LE:  result = (val1 <= val2); break;
                    case Instruction::GE:  result = (val1 >= val2); break;
                    //    // ע�⣺�߼��롢����IR����ͨ������·��ֵת�����˷�֧��
                    //    // ���ﲻ���ϡ�
                    //case Instruction::AND: result = (val1 && val2); break;
                    //case Instruction::OR:  result = (val1 || val2); break;
                    default:
                        optimized = false; // ����ָ�����Ͳ�����
                        break;
                    }

                    if (optimized) {
                        // ԭ���޸�ָ�
                        // 1. �������Ϊ ASSIGN
                        // 2. arg1 ��Ϊ�������ĳ���
                        // 3. arg2 �ÿ�
                        instr.opcode = Instruction::ASSIGN;
                        instr.arg1.kind = Operand::CONST;
                        instr.arg1.value = result;
                        instr.arg2.kind = Operand::NONE;
                    }
                }
            }
        }
    }
}
