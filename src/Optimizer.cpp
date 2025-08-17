#include "Optimizer.hpp"
#include "IRGenerator.hpp" 
#include "ControlFlowGraph.hpp" // 新增：引入CFG头文件
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>
#include <queue>
#include <sstream>
#include <algorithm> // For std::reverse and std::max

// --- 辅助函数 (保持不变) ---

static bool is_variable_or_temp(const Operand& op) {
    return op.kind == Operand::VAR || op.kind == Operand::TEMP;
}

static std::string get_operand_id(const Operand& op) {
    if (op.kind == Operand::VAR) return op.name;
    if (op.kind == Operand::TEMP) return "t" + std::to_string(op.id);
    // 【修复】增加对常量操作数的处理
    if (op.kind == Operand::CONST) return std::to_string(op.value);
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

    bool changed_in_cycle = true;
    while (changed_in_cycle) {
        bool changed_this_pass = false;

        // 1. 结构性与清理优化
        // 将尾递归消除移入循环，让它能从其他优化中受益。
        // 它作为一种强大的结构性改变，适合放在一轮的开始。
        changed_this_pass |= run_tail_recursion_elimination(module);
        changed_this_pass |= run_unreachable_code_elimination(module);

        // 2. 核心简化阶段：传播值，然后化简表达式
        changed_this_pass |= run_constant_propagation(module);
        changed_this_pass |= run_copy_propagation(module);
        changed_this_pass |= run_algebraic_simplification(module);
        changed_this_pass |= run_common_subexpression_elimination(module);

        // 3. 最终清理阶段：移除所有因简化而产生的无用代码
        changed_this_pass |= run_dead_code_elimination(module);

        changed_in_cycle = changed_this_pass;
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
bool Optimizer::run_unreachable_code_elimination(ModuleIR& module) {
    bool changed_at_all = false;
    for (auto& func : module.functions) {
        if (func.blocks.empty()) continue;

        // 1. 构建CFG以获取块之间的连接关系
        ControlFlowGraph cfg(func);
        const auto& nodes = cfg.get_nodes();
        if (nodes.empty()) continue;

        // 创建一个从 BasicBlock* 到 CFGNode* 的映射，方便查找后继
        std::unordered_map<const BasicBlock*, const CFGNode*> block_to_node_map;
        for (const auto& node : nodes) {
            block_to_node_map[node.block] = &node;
        }

        // 2. 使用广度优先搜索（BFS）找出所有可达块
        std::unordered_set<const BasicBlock*> reachable_blocks;
        std::queue<const BasicBlock*> worklist;

        // 从入口块开始
        const BasicBlock* entry_block = &func.blocks.front();
        worklist.push(entry_block);
        reachable_blocks.insert(entry_block);

        while (!worklist.empty()) {
            const BasicBlock* current_block = worklist.front();
            worklist.pop();

            const CFGNode* current_node = block_to_node_map.at(current_block);
            for (const auto* successor_node : current_node->succs) {
                const BasicBlock* successor_block = successor_node->block;
                // 如果后继节点没被访问过，就加入队列和可达集合
                if (reachable_blocks.find(successor_block) == reachable_blocks.end()) {
                    reachable_blocks.insert(successor_block);
                    worklist.push(successor_block);
                }
            }
        }

        // 3. 重建函数的基本块列表，只保留可达的块
        std::vector<BasicBlock> new_blocks;
        for (const auto& block : func.blocks) {
            if (reachable_blocks.count(&block)) {
                new_blocks.push_back(block);
            }
            else {
                changed_at_all = true; // 发现了要删除的块
            }
        }

        // 如果有变化，则更新函数的基本块列表
        if (changed_at_all) {
            func.blocks = std::move(new_blocks);
        }
    }
    return changed_at_all;
}

bool Optimizer::run_constant_propagation(ModuleIR& module) {
    bool changed_at_all = false;
    for (auto& func : module.functions) {
        if (func.blocks.empty()) continue;

        // 1. 构建CFG并运行数据流分析
        ControlFlowGraph cfg(func);
        ConstantPropagationAnalyzer analyzer;
        analyzer.run(func, cfg);
        const auto& in_states = analyzer.get_in_states();

        // 2. 根据分析结果进行代码转换
        for (auto& block : func.blocks) {
            // 获取该块入口处的常量信息
            ConstState current_state = in_states.at(&block);

            // 使用迭代器遍历指令，因为我们可能会删除指令
            for (auto it = block.instructions.begin(); it != block.instructions.end(); /* no increment here */) {
                auto& instr = *it;
                bool instruction_removed = false;

                // 2.1 替换源操作数为常量
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

                // --- 【核心修改：分支简化，适配 JUMP_IF_*】 ---
                // 2.2 如果是条件跳转，且条件是常量，则简化它
                if ((instr.opcode == Instruction::JUMP_IF_ZERO || instr.opcode == Instruction::JUMP_IF_NZERO) && instr.arg1.kind == Operand::CONST) {
                    bool condition_is_zero = (instr.arg1.value == 0);
                    bool jump_on_zero = (instr.opcode == Instruction::JUMP_IF_ZERO);

                //    // 判断跳转是否总是发生
                    if (condition_is_zero == jump_on_zero) {
                        // 条件满足，总是跳转：转换为无条件 JUMP
                        instr.opcode = Instruction::JUMP;
                        // 假设跳转目标在 result 操作数中
                        instr.arg1 = instr.arg2;
                        instr.arg2 = {};
                        changed_at_all = true;
                    }
                    else {
                        // 条件不满足，永不跳转：直接移除该指令
                        it = block.instructions.erase(it);
                        instruction_removed = true;
                        changed_at_all = true;
                    }
                }
                // --- 【修改结束】 ---

                // 2.3 更新当前块内的常量状态 (KILL & GEN)
                // 仅在指令未被移除时执行
                if (!instruction_removed) {
                    if (is_variable_or_temp(instr.result)) {
                        std::string dest_id = get_operand_id(instr.result);
                        current_state.erase(dest_id); // KILL: 任何赋值都会让旧的常量状态失效

                        // GEN: 如果是常量赋值或可折叠的指令，生成新的常量信息
                        if (instr.arg1.kind == Operand::CONST) {
                            if (instr.opcode == Instruction::ASSIGN) {
                                current_state[dest_id] = instr.arg1.value;
                            }
                            else if (instr.arg2.kind == Operand::CONST) {
                                // 源操作数已被替换，可以进行常量折叠
                                int val1 = instr.arg1.value;
                                int val2 = instr.arg2.value;
                                int result_val = 0;
                                bool folded = true;
                                switch (instr.opcode) {
                                case Instruction::ADD: result_val = val1 + val2; break;
                                case Instruction::SUB: result_val = val1 - val2; break;
                                case Instruction::MUL: result_val = val1 * val2; break;
                                case Instruction::DIV: if (val2 == 0) { folded = false; }
                                                     else { result_val = val1 / val2; } break;
                                case Instruction::MOD: if (val2 == 0) { folded = false; }
                                                     else { result_val = val1 % val2; } break;
                                case Instruction::EQ:  result_val = (val1 == val2); break;
                                case Instruction::NEQ: result_val = (val1 != val2); break;
                                case Instruction::LT:  result_val = (val1 < val2);  break;
                                case Instruction::GT:  result_val = (val1 > val2);  break;
                                case Instruction::LE:  result_val = (val1 <= val2); break;
                                case Instruction::GE:  result_val = (val1 >= val2); break;
                                default: folded = false; break;
                                }
                                if (folded) {
                                    current_state[dest_id] = result_val;
                                    // 进一步优化：直接将指令替换为 a = Constant
                                    instr.opcode = Instruction::ASSIGN;
                                    instr.arg1.kind = Operand::CONST;
                                    instr.arg1.value = result_val;
                                    instr.arg2.kind = Operand::NONE;
                                    changed_at_all = true;
                                }
                            }
                        }
                    }
                }

                // 在循环末尾安全地推进迭代器
                if (!instruction_removed) {
                    ++it;
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
                if (instr.arg1.kind == Operand::CONST) {
                    int val1 = instr.arg1.value;
                    if (instr.opcode == Instruction::ADD&&val1 == 0) {
                        instr.opcode = Instruction::ASSIGN;
                        instr.arg1 = instr.arg2;
                        instr.arg2 = {};
                        changed_this_pass = true;
                    }
                    else if (instr.opcode == Instruction::MUL && val1 == 1) {
                        instr.opcode = Instruction::ASSIGN;
                        instr.arg1 = instr.arg2;
                        instr.arg2 = {};
                        changed_this_pass = true;
                    }
                    else if ((instr.opcode == Instruction::MUL || instr.opcode == Instruction::DIV) && val1 == 0) {
                        instr.opcode = Instruction::ASSIGN;
                        instr.arg1.kind = Operand::CONST;
                        instr.arg1.value = 0;
                        instr.arg2 = {};
                        changed_this_pass = true;
                    }
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

        // 1. 分析阶段：运行增强后的分析器
        ControlFlowGraph cfg(func);
        AvailableExpressionsAnalyzer analyzer;
        analyzer.run(func, cfg);
        const auto& in_states = analyzer.get_in_states();

        // 2. 转换阶段
        for (auto& block : func.blocks) {
            // 获取块入口处可用的表达式及其结果
            AvailableExprsMap available_now = in_states.at(&block);

            for (auto& instr : block.instructions) {
                // a. 检查当前指令的表达式是否已经可用
                if (instr.opcode >= Instruction::ADD && instr.opcode <= Instruction::GE) {
                    std::string expr_id = get_expr_id(instr);
                    if (available_now.count(expr_id)) {
                        // 如果可用，直接替换为赋值指令
                        Operand original_result = available_now.at(expr_id);
                        instr.opcode = Instruction::ASSIGN;
                        instr.arg1 = original_result;
                        instr.arg2.kind = Operand::NONE;
                        changed_at_all = true;
                    }
                }

                // b. 更新当前点（块内）的可用表达式信息 (KILL & GEN)
                if (is_variable_or_temp(instr.result)) {
                    std::string dest_id = get_operand_id(instr.result);
                    std::vector<std::string> exprs_to_kill;
                    for (auto const& [expr, op] : available_now) {
                        if (get_operand_id(op) == dest_id || expr.find(dest_id) != std::string::npos) {
                            exprs_to_kill.push_back(expr);
                        }
                    }
                    for (const auto& expr : exprs_to_kill) {
                        available_now.erase(expr);
                    }
                }
                if (instr.opcode >= Instruction::ADD && instr.opcode <= Instruction::GE) {
                    available_now[get_expr_id(instr)] = instr.result;
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
                        // 增加CALL指令。基于“函数是纯的”这一假设，
                        // 如果返回值没被使用，调用本身就是死代码。
                        case Instruction::CALL:
                            is_dead = true;
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
