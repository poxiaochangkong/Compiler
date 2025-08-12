#include "Optimizer.hpp"

// 引入 Instruction::OpCode, Operand::Kind 等枚举
#include "IRGenerator.hpp" 

void Optimizer::optimize(ModuleIR& module) {
    // 遍历模块中的每一个函数
    for (auto& func : module.functions) {
        // 遍历函数中的每一个基本块
        for (auto& block : func.blocks) {
            // 遍历基本块中的每一条指令
            // 我们使用引用(auto& instr)，这样可以直接修改原始指令
            for (auto& instr : block.instructions) {

                // 只处理有 arg1 和 arg2 的指令
                if (instr.arg1.kind == Operand::NONE) {
                    continue;
                }

                // --- 优化点 1: 单操作数指令 (例如 NOT) ---
                if (instr.arg2.kind == Operand::NONE) {
                    if (instr.arg1.kind == Operand::CONST) {
                        int val = instr.arg1.value;
                        int result = 0;
                        bool optimized = true;

                        if (instr.opcode == Instruction::NOT) {
                            result = !val;
                        }
                        else {
                            optimized = false; // 其他单操作数指令暂不处理
                        }

                        if (optimized) {
                            // 原地修改指令：
                            // 1. 操作码变为 ASSIGN
                            // 2. arg1 变为计算结果的常量
                            instr.opcode = Instruction::ASSIGN;
                            instr.arg1.kind = Operand::CONST;
                            instr.arg1.value = result;
                        }
                    }
                    continue; // 处理下一条指令
                }


                // --- 优化点 2: 双操作数指令 (ADD, SUB, LT, etc.) ---
                // 检查两个操作数是否都是常量
                if (instr.arg1.kind == Operand::CONST && instr.arg2.kind == Operand::CONST) {
                    int val1 = instr.arg1.value;
                    int val2 = instr.arg2.value;
                    int result = 0;
                    bool optimized = true; // 标记是否成功优化

                    switch (instr.opcode) {
                    case Instruction::ADD: result = val1 + val2; break;
                    case Instruction::SUB: result = val1 - val2; break;
                    case Instruction::MUL: result = val1 * val2; break;
                    case Instruction::DIV:
                        if (val2 == 0) { optimized = false; break; } // 除零，放弃优化
                        result = val1 / val2;
                        break;
                    case Instruction::MOD:
                        if (val2 == 0) { optimized = false; break; } // 除零，放弃优化
                        result = val1 % val2;
                        break;
                    case Instruction::EQ:  result = (val1 == val2); break;
                    case Instruction::NEQ: result = (val1 != val2); break;
                    case Instruction::LT:  result = (val1 < val2);  break;
                    case Instruction::GT:  result = (val1 > val2);  break;
                    case Instruction::LE:  result = (val1 <= val2); break;
                    case Instruction::GE:  result = (val1 >= val2); break;
                    //    // 注意：逻辑与、或在IR层面通常被短路求值转换成了分支，
                    //    // 这里不加上。
                    //case Instruction::AND: result = (val1 && val2); break;
                    //case Instruction::OR:  result = (val1 || val2); break;
                    default:
                        optimized = false; // 其他指令类型不处理
                        break;
                    }

                    if (optimized) {
                        // 原地修改指令：
                        // 1. 操作码变为 ASSIGN
                        // 2. arg1 变为计算结果的常量
                        // 3. arg2 置空
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
