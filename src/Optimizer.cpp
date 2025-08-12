#include "Optimizer.hpp"
#include "IRGenerator.hpp" 
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>
#include <sstream>

// --- 辅助函数 ---

static bool is_variable_or_temp(const Operand& op) {
    return op.kind == Operand::VAR || op.kind == Operand::TEMP;
}

static std::string get_operand_id(const Operand& op) {
    if (op.kind == Operand::VAR) return op.name;
    if (op.kind == Operand::TEMP) return "t" + std::to_string(op.id);
    return "";
}

// 为一个表达式生成唯一的字符串表示
static std::string get_expr_id(const Instruction& instr) {
    std::stringstream ss;
    ss << instr.opcode;
    if (is_variable_or_temp(instr.arg1)) ss << " " << get_operand_id(instr.arg1);
    else if (instr.arg1.kind == Operand::CONST) ss << " " << instr.arg1.value;

    if (instr.arg2.kind != Operand::NONE) {
        if (is_variable_or_temp(instr.arg2)) ss << " " << get_operand_id(instr.arg2);
        else if (instr.arg2.kind == Operand::CONST) ss << " " << instr.arg2.value;
    }
    return ss.str();
}


// --- 主协调函数 ---

void Optimizer::optimize(ModuleIR& module) {
    run_constant_folding(module);

    bool changed_in_cycle = true;
    while (changed_in_cycle) {
        bool cse_changed = run_common_subexpression_elimination(module);
        bool cp_changed = run_copy_propagation(module);
        bool dce_changed = run_dead_code_elimination(module);

        changed_in_cycle = cse_changed || cp_changed || dce_changed;
    }
}


// --- 优化阶段一: 常量折叠 ---

void Optimizer::run_constant_folding(ModuleIR& module) {
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

// --- 优化阶段二: 公共子表达式消除 (修复版) ---

bool Optimizer::run_common_subexpression_elimination(ModuleIR& module) {
    bool changed_this_pass = false;
    for (auto& func : module.functions) {
        for (auto& block : func.blocks) {
            std::unordered_map<std::string, Operand> available_exprs;
            // 存储每个表达式的操作数，用于精确的失效判断
            std::unordered_map<std::string, std::pair<std::string, std::string>> expr_operands;

            for (auto& instr : block.instructions) {
                // 只处理纯计算指令
                if (instr.opcode >= Instruction::ADD && instr.opcode <= Instruction::GE) {
                    std::string expr_id = get_expr_id(instr);
                    auto it = available_exprs.find(expr_id);

                    if (it != available_exprs.end()) {
                        instr.opcode = Instruction::ASSIGN;
                        instr.arg1 = it->second;
                        instr.arg2.kind = Operand::NONE;
                        changed_this_pass = true;
                    }
                    else {
                        available_exprs[expr_id] = instr.result;
                        // 记录该表达式的两个操作数ID
                        expr_operands[expr_id] = { get_operand_id(instr.arg1), get_operand_id(instr.arg2) };
                    }
                }

                // ---【核心修复】---
                // 如果指令修改了一个变量，需要精确地使所有用到这个变量的旧表达式失效
                if (is_variable_or_temp(instr.result)) {
                    std::string result_id = get_operand_id(instr.result);
                    for (auto it = available_exprs.begin(); it != available_exprs.end(); ) {
                        auto operands_it = expr_operands.find(it->first);
                        if (operands_it != expr_operands.end()) {
                            // 精确判断：被修改的变量是否是表达式的操作数之一
                            if (operands_it->second.first == result_id || operands_it->second.second == result_id) {
                                // 是，则该表达式失效
                                expr_operands.erase(operands_it);
                                it = available_exprs.erase(it);
                                continue;
                            }
                        }
                        ++it;
                    }
                }
            }
        }
    }
    return changed_this_pass;
}

// --- 优化阶段三: 复写传播 (局部) ---

bool Optimizer::run_copy_propagation(ModuleIR& module) {
    bool changed_this_pass = false;
    for (auto& func : module.functions) {
        for (auto& block : func.blocks) {
            std::unordered_map<std::string, Operand> copy_map;
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

                if (is_variable_or_temp(instr.result)) {
                    std::string result_id = get_operand_id(instr.result);
                    copy_map.erase(result_id);
                    for (auto it = copy_map.begin(); it != copy_map.end(); ) {
                        if (get_operand_id(it->second) == result_id) {
                            it = copy_map.erase(it);
                        }
                        else {
                            ++it;
                        }
                    }
                }

                if (instr.opcode == Instruction::ASSIGN && is_variable_or_temp(instr.result) && is_variable_or_temp(instr.arg1)) {
                    if (get_operand_id(instr.result) != get_operand_id(instr.arg1)) {
                        copy_map[get_operand_id(instr.result)] = instr.arg1;
                    }
                }
            }
        }
    }
    return changed_this_pass;
}


// --- 优化阶段四: 死代码消除 ---

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
