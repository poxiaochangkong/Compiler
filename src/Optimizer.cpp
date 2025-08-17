#include "Optimizer.hpp"
#include "IRGenerator.hpp" 
#include "ControlFlowGraph.hpp" // ����������CFGͷ�ļ�
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>
#include <queue>
#include <sstream>
#include <algorithm> // For std::reverse and std::max

// --- �������� (���ֲ���) ---

static bool is_variable_or_temp(const Operand& op) {
    return op.kind == Operand::VAR || op.kind == Operand::TEMP;
}

static std::string get_operand_id(const Operand& op) {
    if (op.kind == Operand::VAR) return op.name;
    if (op.kind == Operand::TEMP) return "t" + std::to_string(op.id);
    // ���޸������ӶԳ����������Ĵ���
    if (op.kind == Operand::CONST) return std::to_string(op.value);
    return "";
}

static std::string get_expr_id(const Instruction& instr) {
    std::stringstream ss;
    std::string op1_id = get_operand_id(instr.arg1);
    std::string op2_id = get_operand_id(instr.arg2);
    // �Բ���������ȷ�� a+b �� b+a ����ͬ��ID
    if (op1_id > op2_id) std::swap(op1_id, op2_id);

    ss << instr.opcode << " " << op1_id << " " << op2_id;
    return ss.str();
}

// --- ��Э������ (���ֲ���) ---

void Optimizer::optimize(ModuleIR& module) {
    initialize_temp_counter(module);

    bool changed_in_cycle = true;
    while (changed_in_cycle) {
        bool changed_this_pass = false;

        // 1. �ṹ���������Ż�
        // ��β�ݹ���������ѭ���������ܴ������Ż������档
        // ����Ϊһ��ǿ��Ľṹ�Ըı䣬�ʺϷ���һ�ֵĿ�ʼ��
        changed_this_pass |= run_tail_recursion_elimination(module);
        changed_this_pass |= run_unreachable_code_elimination(module);

        // 2. ���ļ򻯽׶Σ�����ֵ��Ȼ�󻯼���ʽ
        changed_this_pass |= run_constant_propagation(module);
        changed_this_pass |= run_copy_propagation(module);
        changed_this_pass |= run_algebraic_simplification(module);
        changed_this_pass |= run_common_subexpression_elimination(module);

        // 3. ��������׶Σ��Ƴ�������򻯶����������ô���
        changed_this_pass |= run_dead_code_elimination(module);

        changed_in_cycle = changed_this_pass;
    }
}


// --- ��ʱ�������� (���ֲ���) ---

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


// --- �����Ż��׶� (ȫ�����ֲ���) ---
bool Optimizer::run_unreachable_code_elimination(ModuleIR& module) {
    bool changed_at_all = false;
    for (auto& func : module.functions) {
        if (func.blocks.empty()) continue;

        // 1. ����CFG�Ի�ȡ��֮������ӹ�ϵ
        ControlFlowGraph cfg(func);
        const auto& nodes = cfg.get_nodes();
        if (nodes.empty()) continue;

        // ����һ���� BasicBlock* �� CFGNode* ��ӳ�䣬������Һ��
        std::unordered_map<const BasicBlock*, const CFGNode*> block_to_node_map;
        for (const auto& node : nodes) {
            block_to_node_map[node.block] = &node;
        }

        // 2. ʹ�ù������������BFS���ҳ����пɴ��
        std::unordered_set<const BasicBlock*> reachable_blocks;
        std::queue<const BasicBlock*> worklist;

        // ����ڿ鿪ʼ
        const BasicBlock* entry_block = &func.blocks.front();
        worklist.push(entry_block);
        reachable_blocks.insert(entry_block);

        while (!worklist.empty()) {
            const BasicBlock* current_block = worklist.front();
            worklist.pop();

            const CFGNode* current_node = block_to_node_map.at(current_block);
            for (const auto* successor_node : current_node->succs) {
                const BasicBlock* successor_block = successor_node->block;
                // �����̽ڵ�û�����ʹ����ͼ�����кͿɴＯ��
                if (reachable_blocks.find(successor_block) == reachable_blocks.end()) {
                    reachable_blocks.insert(successor_block);
                    worklist.push(successor_block);
                }
            }
        }

        // 3. �ؽ������Ļ������б�ֻ�����ɴ�Ŀ�
        std::vector<BasicBlock> new_blocks;
        for (const auto& block : func.blocks) {
            if (reachable_blocks.count(&block)) {
                new_blocks.push_back(block);
            }
            else {
                changed_at_all = true; // ������Ҫɾ���Ŀ�
            }
        }

        // ����б仯������º����Ļ������б�
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

        // 1. ����CFG����������������
        ControlFlowGraph cfg(func);
        ConstantPropagationAnalyzer analyzer;
        analyzer.run(func, cfg);
        const auto& in_states = analyzer.get_in_states();

        // 2. ���ݷ���������д���ת��
        for (auto& block : func.blocks) {
            // ��ȡ�ÿ���ڴ��ĳ�����Ϣ
            ConstState current_state = in_states.at(&block);

            // ʹ�õ���������ָ���Ϊ���ǿ��ܻ�ɾ��ָ��
            for (auto it = block.instructions.begin(); it != block.instructions.end(); /* no increment here */) {
                auto& instr = *it;
                bool instruction_removed = false;

                // 2.1 �滻Դ������Ϊ����
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

                // --- �������޸ģ���֧�򻯣����� JUMP_IF_*�� ---
                // 2.2 �����������ת���������ǳ����������
                if ((instr.opcode == Instruction::JUMP_IF_ZERO || instr.opcode == Instruction::JUMP_IF_NZERO) && instr.arg1.kind == Operand::CONST) {
                    bool condition_is_zero = (instr.arg1.value == 0);
                    bool jump_on_zero = (instr.opcode == Instruction::JUMP_IF_ZERO);

                //    // �ж���ת�Ƿ����Ƿ���
                    if (condition_is_zero == jump_on_zero) {
                        // �������㣬������ת��ת��Ϊ������ JUMP
                        instr.opcode = Instruction::JUMP;
                        // ������תĿ���� result ��������
                        instr.arg1 = instr.arg2;
                        instr.arg2 = {};
                        changed_at_all = true;
                    }
                    else {
                        // ���������㣬������ת��ֱ���Ƴ���ָ��
                        it = block.instructions.erase(it);
                        instruction_removed = true;
                        changed_at_all = true;
                    }
                }
                // --- ���޸Ľ����� ---

                // 2.3 ���µ�ǰ���ڵĳ���״̬ (KILL & GEN)
                // ����ָ��δ���Ƴ�ʱִ��
                if (!instruction_removed) {
                    if (is_variable_or_temp(instr.result)) {
                        std::string dest_id = get_operand_id(instr.result);
                        current_state.erase(dest_id); // KILL: �κθ�ֵ�����þɵĳ���״̬ʧЧ

                        // GEN: ����ǳ�����ֵ����۵���ָ������µĳ�����Ϣ
                        if (instr.arg1.kind == Operand::CONST) {
                            if (instr.opcode == Instruction::ASSIGN) {
                                current_state[dest_id] = instr.arg1.value;
                            }
                            else if (instr.arg2.kind == Operand::CONST) {
                                // Դ�������ѱ��滻�����Խ��г����۵�
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
                                    // ��һ���Ż���ֱ�ӽ�ָ���滻Ϊ a = Constant
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

                // ��ѭ��ĩβ��ȫ���ƽ�������
                if (!instruction_removed) {
                    ++it;
                }
            }
        }
    }
    return changed_at_all;
}


bool Optimizer::run_algebraic_simplification(ModuleIR& module) {
    // ... (�����ޱ仯������ԭ��)
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

        // 1. �����׶Σ�������ǿ��ķ�����
        ControlFlowGraph cfg(func);
        AvailableExpressionsAnalyzer analyzer;
        analyzer.run(func, cfg);
        const auto& in_states = analyzer.get_in_states();

        // 2. ת���׶�
        for (auto& block : func.blocks) {
            // ��ȡ����ڴ����õı��ʽ������
            AvailableExprsMap available_now = in_states.at(&block);

            for (auto& instr : block.instructions) {
                // a. ��鵱ǰָ��ı��ʽ�Ƿ��Ѿ�����
                if (instr.opcode >= Instruction::ADD && instr.opcode <= Instruction::GE) {
                    std::string expr_id = get_expr_id(instr);
                    if (available_now.count(expr_id)) {
                        // ������ã�ֱ���滻Ϊ��ֵָ��
                        Operand original_result = available_now.at(expr_id);
                        instr.opcode = Instruction::ASSIGN;
                        instr.arg1 = original_result;
                        instr.arg2.kind = Operand::NONE;
                        changed_at_all = true;
                    }
                }

                // b. ���µ�ǰ�㣨���ڣ��Ŀ��ñ��ʽ��Ϣ (KILL & GEN)
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
    // ... (�����ޱ仯������ԭ��)
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

// --- �Ż��׶���: ���������� (ȫ��ʵ��) ---
bool Optimizer::run_dead_code_elimination(ModuleIR& module) {
    bool changed_at_all = false;
    // ��ÿ�������������з������Ż�
    for (auto& func : module.functions) {
        if (func.blocks.empty()) continue;

        // 1. ����CFG�����л�Ծ��������
        // �������ֻ��DCE�ڲ�������Ӱ�������Ż�
        ControlFlowGraph cfg(func);
        cfg.run_liveness_analysis();

        // 2. ʹ��һ��map�����ٲ���ÿ��block��Ӧ��CFGNode�������
        std::unordered_map<const BasicBlock*, const CFGNode*> block_to_node;
        for (const auto& node : cfg.get_nodes()) {
            block_to_node[node.block] = &node;
        }

        // 3. ���������е�ÿ�������飬�����÷����������DCE
        for (auto& block : func.blocks) {
            // ��ȡ��ǰ��Ļ�Ծ�����������
            const CFGNode* current_node = block_to_node.at(&block);
            // �ӿ�ĳ��ڴ��Ļ�Ծ������ʼ�������Ƶ�
            std::set<std::string> live_now = current_node->live_out;

            // ������Ҫ�Ӻ���ǰ����������ɾ��ָ�����ʹ��һ����ʱ��vector
            std::vector<Instruction> new_instructions;
            new_instructions.reserve(block.instructions.size());

            for (auto it = block.instructions.rbegin(); it != block.instructions.rend(); ++it) {
                const auto& instr = *it;
                bool is_dead = false;

                if (is_variable_or_temp(instr.result)) {
                    std::string result_id = get_operand_id(instr.result);
                    // �����жϣ����һ��ָ��Ľ�����ڵ�ǰ��Ծ���������У�
                    // ���Ҹ�ָ��û�и����ã��纯�����ã�����Ϊ�����롣
                    if (live_now.find(result_id) == live_now.end()) {
                        switch (instr.opcode) {
                        case Instruction::ADD: case Instruction::SUB: case Instruction::MUL:
                        case Instruction::DIV: case Instruction::MOD: case Instruction::NOT:
                        case Instruction::EQ:  case Instruction::NEQ: case Instruction::LT:
                        case Instruction::GT:  case Instruction::LE:  case Instruction::GE:
                        case Instruction::ASSIGN:
                            is_dead = true;
                            break;
                        // ����CALLָ����ڡ������Ǵ��ġ���һ���裬
                        // �������ֵû��ʹ�ã����ñ�����������롣
                        case Instruction::CALL:
                            is_dead = true;
                        default: // CALL, RET, JUMP���и����õ�ָ��ܱ�����
                            is_dead = false;
                            break;
                        }
                    }
                }

                if (is_dead) {
                    changed_at_all = true;
                }
                else {
                    // ���ָ������ģ�������
                    new_instructions.push_back(instr);
                    // �����»�Ծ�������ϣ��Ƴ�����ı��������ʹ�õı���
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
            // ��Ϊ�����Ƿ��������push_back�����������Ҫ�ٴη�ת
            std::reverse(new_instructions.begin(), new_instructions.end());
            block.instructions = std::move(new_instructions);
        }
    }
    return changed_at_all;
}


// --- �Ż��׶���: β�ݹ����� (���ֲ���) ---

bool Optimizer::run_tail_recursion_elimination(ModuleIR& module) {
    // ... (�����ޱ仯������ԭ��)
    bool changed_this_pass = false;
    for (auto& func : module.functions) {
        if (func.blocks.empty()) continue;

        // ���޸���1. �ƶϺ����Ƿ�Ϊ non-void
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

            // ���޸���2. ʹ���ƶϳ���������Ϣ���м��
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
