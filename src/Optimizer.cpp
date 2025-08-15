#include "Optimizer.hpp"
#include "IRGenerator.hpp" 
#include "ControlFlowGraph.hpp" // 新增：引入CFG头文件
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm> // For std::reverse and std::max

// --- 辅助函数 (保持不变) ---

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
    std::string op1_id = get_operand_id(instr.arg1);
    std::string op2_id = get_operand_id(instr.arg2);
    // 对操作数排序，确保 a+b 和 b+a 有相同的ID
    if (op1_id > op2_id) std::swap(op1_id, op2_id);

    ss << instr.opcode << " " << op1_id << " " << op2_id;
    return ss.str();
}

// --- 主协调函数 (保持不变) ---

void Optimizer::optimize(ModuleIR& module) {
    initialize_temp_counter(module);

    // 尾递归消除通常只做一次，且在循环优化前做比较好
    run_tail_recursion_elimination(module);

    bool changed_in_cycle = true;
    while (changed_in_cycle) {
        // 【新增】将常量传播加入优化循环
        bool cp_prop_changed = run_constant_propagation(module);
        bool alg_changed = run_algebraic_simplification(module);
        bool cse_changed = run_common_subexpression_elimination(module);
        bool copy_prop_changed = run_copy_propagation(module);
        bool dce_changed = run_dead_code_elimination(module);

        changed_in_cycle = cp_prop_changed || alg_changed || cse_changed || copy_prop_changed || dce_changed;
    }
}

// --- 临时变量工具 (保持不变) ---

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


// --- 其他优化阶段 (全部保持不变) ---
bool Optimizer::run_constant_propagation(ModuleIR& module) {
    bool changed_at_all = false;
    for (auto& func : module.functions) {
        if (func.blocks.empty()) continue;

        // 1. 构建CFG
        ControlFlowGraph cfg(func);

        // 2. 运行分析
        ConstantPropagationAnalyzer analyzer;
        analyzer.run(func, cfg);

        // 3. 获取分析结果
        const auto& in_states = analyzer.get_in_states();

        // 4. 根据分析结果进行代码转换
        for (auto& block : func.blocks) {
            // 获取该块入口处的常量信息，并用它来维护块内的当前状态
            ConstState current_state = in_states.at(&block);

            for (auto& instr : block.instructions) {
                // 4.1 替换源操作数
                Operand* operands_to_check[] = { &instr.arg1, &instr.arg2 };
                for (auto* op_ptr : operands_to_check) {
                    if (is_variable_or_temp(*op_ptr)) {
                        std::string id = get_operand_id(*op_ptr);
                        if (current_state.count(id)) {
                            op_ptr->kind = Operand::CONST;
                            op_ptr->value = current_state.at(id);
                            changed_at_all = true;
                        }
                    }
                }

                // 4.2 更新当前块内的常量状态 (KILL & GEN)
                if (is_variable_or_temp(instr.result)) {
                    std::string dest_id = get_operand_id(instr.result);
                    current_state.erase(dest_id); // KILL: 赋值让旧状态失效

                    // GEN: 如果是常量赋值或可折叠的指令，生成新的常量信息
                    if (instr.arg1.kind == Operand::CONST) {
                        if (instr.opcode == Instruction::ASSIGN) {
                            current_state[dest_id] = instr.arg1.value;
                        }
                        else if (instr.arg2.kind == Operand::CONST) {
                            // 此时源操作数已经被替换，可以进行常量折叠
                            int val1 = instr.arg1.value;
                            int val2 = instr.arg2.value;
                            int result = 0;
                            bool folded = true;
                            switch (instr.opcode) {
                            case Instruction::ADD: result = val1 + val2; break;
                            case Instruction::SUB: result = val1 - val2; break;
                            case Instruction::MUL: result = val1 * val2; break;
                            case Instruction::DIV: if (val2 == 0) { folded = false; break; } result = val1 / val2; break;
                            case Instruction::MOD: if (val2 == 0) { folded = false; break; } result = val1 % val2; break;
                            case Instruction::EQ:  result = (val1 == val2); break;
                            case Instruction::NEQ: result = (val1 != val2); break;
                            case Instruction::LT:  result = (val1 < val2);  break;
                            case Instruction::GT:  result = (val1 > val2);  break;
                            case Instruction::LE:  result = (val1 <= val2); break;
                            case Instruction::GE:  result = (val1 >= val2); break;
                            default: folded = false; break;
                            }
                            if (folded) {
                                current_state[dest_id] = result;
                                // 进一步优化：直接将指令替换为 a = C
                                instr.opcode = Instruction::ASSIGN;
                                instr.arg1.value = result;
                                instr.arg2.kind = Operand::NONE;
                                changed_at_all = true;
                            }
                        }
                    }
                }
            }
        }
    }
    return changed_at_all;
}

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
                    }
                    else if ((instr.opcode == Instruction::MUL || instr.opcode == Instruction::DIV) && val == -1) {
                        instr.opcode = Instruction::ASSIGN;
                        instr.arg2.kind = Operand::NONE;
                        instr.arg1.value = 0 - instr.arg1.value;
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

bool Optimizer::run_common_subexpression_elimination(ModuleIR& module) {
    bool changed_at_all = false;
    for (auto& func : module.functions) {
        if (func.blocks.empty()) continue;

        // 1. 分析阶段：运行可用表达式分析器
        ControlFlowGraph cfg(func);
        AvailableExpressionsAnalyzer analyzer;
        analyzer.run(func, cfg);
        const auto& in_states = analyzer.get_in_states();

        // 2. 转换阶段
        //    这个map用于在遍历过程中，追踪表达式的计算结果被存放在哪个操作数里
        std::unordered_map<std::string, Operand> expr_result_map;

        for (auto& block : func.blocks) {
            // 获取在此块入口处就可用的表达式集合
            const auto& available_on_entry = in_states.at(&block);

            // 遍历块内指令
            for (auto& instr : block.instructions) {
                // a. 检查当前指令是否是一个可以被消除的表达式
                if (instr.opcode >= Instruction::ADD && instr.opcode <= Instruction::GE) {
                    std::string expr_id = get_expr_id(instr);

                    // 如果表达式在块入口可用，并且我们已经记录了它的计算结果
                    if (available_on_entry.count(expr_id) && expr_result_map.count(expr_id)) {
                        // 找到了一个全局公共子表达式！
                        Operand original_result = expr_result_map.at(expr_id);

                        // 将当前指令替换为简单的赋值
                        instr.opcode = Instruction::ASSIGN;
                        instr.arg1 = original_result;
                        instr.arg2.kind = Operand::NONE;
                        changed_at_all = true;
                    }
                    else {
                        // 这是一个新的计算，记录下它的结果
                        expr_result_map[expr_id] = instr.result;
                    }
                }

                // b. 失效(Kill)逻辑：如果指令修改了一个变量，
                //    那么所有使用这个变量作为操作数的表达式都变得不再可用。
                if (is_variable_or_temp(instr.result)) {
                    std::string result_id = get_operand_id(instr.result);
                    for (auto it = expr_result_map.begin(); it != expr_result_map.end(); ) {
                        // 检查表达式的操作数是否包含被修改的变量
                        // 注意：这是一个简化的检查，它假定表达式字符串包含操作数ID
                        if (it->first.find(result_id) != std::string::npos) {
                            it = expr_result_map.erase(it);
                        }
                        else {
                            ++it;
                        }
                    }
                }
            }
        }
    }
    return changed_at_all;
}

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

// --- 优化阶段五: 死代码消除 (全新实现) ---
bool Optimizer::run_dead_code_elimination(ModuleIR& module) {
    bool changed_at_all = false;
    // 对每个函数独立进行分析和优化
    for (auto& func : module.functions) {
        if (func.blocks.empty()) continue;

        // 1. 构建CFG并运行活跃变量分析
        // 这个开销只在DCE内部，不会影响其他优化
        ControlFlowGraph cfg(func);
        cfg.run_liveness_analysis();

        // 2. 使用一个map来快速查找每个block对应的CFGNode分析结果
        std::unordered_map<const BasicBlock*, const CFGNode*> block_to_node;
        for (const auto& node : cfg.get_nodes()) {
            block_to_node[node.block] = &node;
        }

        // 3. 遍历函数中的每个基本块，并利用分析结果进行DCE
        for (auto& block : func.blocks) {
            // 获取当前块的活跃变量分析结果
            const CFGNode* current_node = block_to_node.at(&block);
            // 从块的出口处的活跃变量开始，反向推导
            std::set<std::string> live_now = current_node->live_out;

            // 我们需要从后往前遍历并可能删除指令，所以使用一个临时的vector
            std::vector<Instruction> new_instructions;
            new_instructions.reserve(block.instructions.size());

            for (auto it = block.instructions.rbegin(); it != block.instructions.rend(); ++it) {
                const auto& instr = *it;
                bool is_dead = false;

                if (is_variable_or_temp(instr.result)) {
                    std::string result_id = get_operand_id(instr.result);
                    // 核心判断：如果一个指令的结果不在当前活跃变量集合中，
                    // 并且该指令没有副作用（如函数调用），则为死代码。
                    if (live_now.find(result_id) == live_now.end()) {
                        switch (instr.opcode) {
                        case Instruction::ADD: case Instruction::SUB: case Instruction::MUL:
                        case Instruction::DIV: case Instruction::MOD: case Instruction::NOT:
                        case Instruction::EQ:  case Instruction::NEQ: case Instruction::LT:
                        case Instruction::GT:  case Instruction::LE:  case Instruction::GE:
                        case Instruction::ASSIGN:
                            is_dead = true;
                            break;
                        default: // CALL, RET, JUMP等有副作用的指令不能被消除
                            is_dead = false;
                            break;
                        }
                    }
                }

                if (is_dead) {
                    changed_at_all = true;
                }
                else {
                    // 如果指令不是死的，保留它
                    new_instructions.push_back(instr);
                    // 并更新活跃变量集合：移除定义的变量，添加使用的变量
                    if (is_variable_or_temp(instr.result)) {
                        live_now.erase(get_operand_id(instr.result));
                    }
                    if (is_variable_or_temp(instr.arg1)) {
                        live_now.insert(get_operand_id(instr.arg1));
                    }
                    if (is_variable_or_temp(instr.arg2)) {
                        live_now.insert(get_operand_id(instr.arg2));
                    }
                }
            }
            // 因为我们是反向遍历并push_back，所以最后需要再次反转
            std::reverse(new_instructions.begin(), new_instructions.end());
            block.instructions = std::move(new_instructions);
        }
    }
    return changed_at_all;
}


// --- 优化阶段六: 尾递归消除 (保持不变) ---

bool Optimizer::run_tail_recursion_elimination(ModuleIR& module) {
    // ... (代码无变化，保持原样)
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
