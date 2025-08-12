#include "Optimizer.hpp"
#include "IRGenerator.hpp" 
#include <unordered_set>
#include <string>
#include <vector>

// --- 辅助函数 ---

// 检查一个操作数是否是变量或临时变量
static bool is_variable_or_temp(const Operand& op) {
    return op.kind == Operand::VAR || op.kind == Operand::TEMP;
}

// 为变量或临时变量生成一个唯一的字符串标识符，用于在集合中追踪
static std::string get_operand_id(const Operand& op) {
    if (op.kind == Operand::VAR) {
        return op.name;
    }
    if (op.kind == Operand::TEMP) {
        return "t" + std::to_string(op.id);
    }
    return ""; // 其他类型操作数没有ID
}


void Optimizer::optimize(ModuleIR& module) {
    // --- 阶段一: 常量折叠 ---
    // (与之前逻辑相同，但移除了不会出现的 AND 和 OR 分支)
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

    // --- 阶段二: 死代码消除 (迭代进行) ---
    bool changed_in_pass = true;
    // 只要上一轮有代码被消除，就继续循环，直到没有代码可消除为止
    while (changed_in_pass) {
        changed_in_pass = false;

        for (auto& func : module.functions) {
            // 1. 收集所有被"使用"的变量和临时变量
            std::unordered_set<std::string> used_operands;
            for (auto& block : func.blocks) {
                for (auto& instr : block.instructions) {
                    // 操作数在 arg1 或 arg2 中出现，即为"使用"
                    if (is_variable_or_temp(instr.arg1)) {
                        used_operands.insert(get_operand_id(instr.arg1));
                    }
                    if (is_variable_or_temp(instr.arg2)) {
                        used_operands.insert(get_operand_id(instr.arg2));
                    }
                    
                    // 特殊使用情况：
                    // - `RET val`: val 被使用
                    // - `JUMP_IF cond`: cond 被使用
                    // - `PARAM p`: p 被使用
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

            // 2. 遍历并移除结果从未被使用的指令
            for (auto& block : func.blocks) {
                std::vector<Instruction> new_instructions;
                new_instructions.reserve(block.instructions.size());

                for (const auto& instr : block.instructions) {
                    bool is_dead = false;
                    // 检查指令是否定义了一个从未被使用的结果
                    if (is_variable_or_temp(instr.result)) {
                        // 如果在"使用"集合中找不到这个结果，它就有可能是死代码
                        if (used_operands.find(get_operand_id(instr.result)) == used_operands.end()) {
                            // 再次确认该指令没有副作用 (Side Effect)
                            switch (instr.opcode) {
                                // 这些指令是纯计算，如果没有人使用其结果，就是死的
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
                                
                                // CALL 指令有副作用，即使返回值不用，也不能删除
                                case Instruction::CALL:
                                default:
                                    is_dead = false;
                                    break;
                            }
                        }
                    }

                    if (is_dead) {
                        changed_in_pass = true; // 标记本轮有代码被删除
                    } else {
                        new_instructions.push_back(instr);
                    }
                }
                // 用新的指令列表替换旧的
                block.instructions = std::move(new_instructions);
            }
        }
    }
}
