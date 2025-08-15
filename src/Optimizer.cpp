#include "Optimizer.hpp"
#include "IRGenerator.hpp" 
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm> // For std::reverse and std::max

// --- 辅助函数 ---

static bool is_variable_or_temp(const Operand& op) {
    return op.kind == Operand::VAR || op.kind == Operand::TEMP;
}

static std::string get_operand_id(const Operand& op) {
    if (op.kind == Operand::VAR) return op.name;
    if (op.kind == Operand::TEMP) return "t" + std::to_string(op.id);
    return "";
}

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
    // 【修复】在所有优化开始前，初始化优化器自己的临时变量计数器
    initialize_temp_counter(module);

    run_constant_folding(module);
    run_tail_recursion_elimination(module);

    bool changed_in_cycle = true;
    while (changed_in_cycle) {
        bool alg_changed = run_algebraic_simplification(module);
        bool cse_changed = run_common_subexpression_elimination(module);
        bool cp_changed = run_copy_propagation(module);
        bool dce_changed = run_dead_code_elimination(module);

        changed_in_cycle = alg_changed || cse_changed || cp_changed || dce_changed;
    }
}

// --- 【修复】用于创建唯一临时变量的内部工具 ---

void Optimizer::initialize_temp_counter(const ModuleIR& module) {
    int max_id = -1;
    for (const auto& func : module.functions) {
        for (const auto& block : func.blocks) {
            for (const auto& instr : block.instructions) {
                if (instr.result.kind == Operand::TEMP) max_id = std::max(max_id, instr.result.id);
                if (instr.arg1.kind == Operand::TEMP)   max_id = std::max(max_id, instr.arg1.id);
                if (instr.arg2.kind == Operand::TEMP)   max_id = std::max(max_id, instr.arg2.id);
            }
        }
    }
    m_temp_counter = max_id + 1;
}

Operand Optimizer::new_temp() {
    Operand op;
    op.kind = Operand::TEMP;
    op.id = m_temp_counter++;
    return op;
}


// --- 优化阶段一: 常量折叠 ---
void Optimizer::run_constant_folding(ModuleIR& module) {
    // ... (代码无变化，保持原样)
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

// --- 优化阶段二: 代数化简 ---
bool Optimizer::run_algebraic_simplification(ModuleIR& module) {
    // ... (代码无变化，保持原样)
    bool changed_this_pass = false;
    for (auto& func : module.functions) {
        for (auto& block : func.blocks) {
            for (auto& instr : block.instructions) {
                if (instr.opcode == Instruction::SUB && get_operand_id(instr.arg1) == get_operand_id(instr.arg2) && is_variable_or_temp(instr.arg1)) {
                    instr.opcode = Instruction::ASSIGN;
                    instr.arg1.kind = Operand::CONST;
                    instr.arg1.value = 0;
                    instr.arg2.kind = Operand::NONE;
                    changed_this_pass = true;
                    continue;
                }
                if (instr.arg2.kind == Operand::CONST) {
                    int val = instr.arg2.value;
                    if ((instr.opcode == Instruction::ADD || instr.opcode == Instruction::SUB) && val == 0) {
                        instr.opcode = Instruction::ASSIGN;
                        instr.arg2.kind = Operand::NONE;
                        changed_this_pass = true;
                    }
                    else if ((instr.opcode == Instruction::MUL || instr.opcode == Instruction::DIV) && val == 1) {
                        instr.opcode = Instruction::ASSIGN;
                        instr.arg2.kind = Operand::NONE;
                        changed_this_pass = true;
                    }else if ((instr.opcode == Instruction::MUL || instr.opcode == Instruction::DIV) && val == -1) {
                        instr.opcode = Instruction::ASSIGN;
                        instr.arg2.kind = Operand::NONE;
                        instr.arg1.value = 0-instr.arg1.value;
                        changed_this_pass = true;
                    }
                    else if (instr.opcode == Instruction::MUL && val == 0) {
                        instr.opcode = Instruction::ASSIGN;
                        instr.arg1.kind = Operand::CONST;
                        instr.arg1.value = 0;
                        instr.arg2.kind = Operand::NONE;
                        changed_this_pass = true;
                    }
                }
            }
        }
    }
    return changed_this_pass;
}

// --- 优化阶段三: 公共子表达式消除 ---
bool Optimizer::run_common_subexpression_elimination(ModuleIR& module) {
    // ... (代码无变化，保持原样)
    bool changed_this_pass = false;
    for (auto& func : module.functions) {
        for (auto& block : func.blocks) {
            std::unordered_map<std::string, Operand> available_exprs;
            std::unordered_map<std::string, std::pair<std::string, std::string>> expr_operands;
            for (auto& instr : block.instructions) {
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
                        expr_operands[expr_id] = { get_operand_id(instr.arg1), get_operand_id(instr.arg2) };
                    }
                }
                if (is_variable_or_temp(instr.result)) {
                    std::string result_id = get_operand_id(instr.result);
                    for (auto it = available_exprs.begin(); it != available_exprs.end(); ) {
                        auto operands_it = expr_operands.find(it->first);
                        if (operands_it != expr_operands.end()) {
                            if (operands_it->second.first == result_id || operands_it->second.second == result_id) {
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

// --- 优化阶段四: 复写传播 ---
bool Optimizer::run_copy_propagation(ModuleIR& module) {
    // ... (代码无变化，保持原样)
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

// --- 优化阶段五: 死代码消除 ---
bool Optimizer::run_dead_code_elimination(ModuleIR& module) {
    // ... (代码无变化，保持原样)
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

// --- 【修复】优化阶段六: 尾递归消除 ---

bool Optimizer::run_tail_recursion_elimination(ModuleIR& module) {
    bool changed_this_pass = false;
    for (auto& func : module.functions) {
        if (func.blocks.empty()) continue;

        // 【修复】1. 推断函数是否为 non-void
        bool is_non_void = false;
        for (const auto& block : func.blocks) {
            for (const auto& instr : block.instructions) {
                if (instr.opcode == Instruction::RET && instr.arg1.kind != Operand::NONE) {
                    is_non_void = true;
                    break;
                }
            }
            if (is_non_void) break;
        }

        for (auto& block : func.blocks) {
            if (block.instructions.size() < 2) continue;

            Instruction& ret_instr = block.instructions.back();
            if (ret_instr.opcode != Instruction::RET) continue;

            Instruction& call_instr = block.instructions[block.instructions.size() - 2];

            if (call_instr.opcode != Instruction::CALL || call_instr.arg1.kind != Operand::LABEL || call_instr.arg1.name != func.name) continue;

            // 【修复】2. 使用推断出的类型信息进行检查
            if (is_non_void) {
                if (ret_instr.arg1.kind == Operand::NONE || get_operand_id(ret_instr.arg1) != get_operand_id(call_instr.result)) {
                    continue;
                }
            }

            std::vector<Operand> new_args;
            size_t param_count = 0;
            for (int i = block.instructions.size() - 3; i >= 0; --i) {
                if (block.instructions[i].opcode == Instruction::PARAM) {
                    new_args.push_back(block.instructions[i].arg1);
                    param_count++;
                }
                else {
                    break;
                }
            }

            if (param_count != func.params.size()) continue;
            std::reverse(new_args.begin(), new_args.end());

            std::vector<Instruction> new_instrs;
            std::vector<Operand> temp_operands;
            for (size_t i = 0; i < func.params.size(); ++i) {
                Operand temp_op = this->new_temp();
                temp_operands.push_back(temp_op);

                Instruction assign_to_temp;
                assign_to_temp.opcode = Instruction::ASSIGN;
                assign_to_temp.result = temp_op;
                assign_to_temp.arg1 = new_args[i];
                new_instrs.push_back(assign_to_temp);
            }

            for (size_t i = 0; i < func.params.size(); ++i) {
                Instruction assign_to_param;
                assign_to_param.opcode = Instruction::ASSIGN;

                Operand param_op;
                param_op.kind = Operand::VAR;
                param_op.name = func.params[i].name;

                assign_to_param.result = param_op;
                assign_to_param.arg1 = temp_operands[i];
                new_instrs.push_back(assign_to_param);
            }

            Instruction jump_instr;
            jump_instr.opcode = Instruction::JUMP;
            jump_instr.arg1.kind = Operand::LABEL;
            jump_instr.arg1.name = func.blocks.front().label;
            new_instrs.push_back(jump_instr);

            block.instructions.erase(block.instructions.begin() + (block.instructions.size() - 2 - param_count), block.instructions.end());
            block.instructions.insert(block.instructions.end(), new_instrs.begin(), new_instrs.end());

            changed_this_pass = true;
            break;
        }
    }
    return changed_this_pass;
}
