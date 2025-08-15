#include "ConstantPropagationAnalyzer.hpp"
#include <vector>
#include <iostream>
#include <unordered_map> // 新增，用于创建映射

// --- 辅助函数 ---

static std::string get_operand_id(const Operand& op) {
    if (op.kind == Operand::VAR) return op.name;
    if (op.kind == Operand::TEMP) return "t" + std::to_string(op.id);
    return "";
}

// --- ConstantPropagationAnalyzer 实现 ---

void ConstantPropagationAnalyzer::run(FunctionIR& func, ControlFlowGraph& cfg) {
    m_in_states.clear();
    m_out_states.clear();

    // 【修复】第一步：建立 BasicBlock* 到 CFGNode* 的映射，方便快速查找
    std::unordered_map<const BasicBlock*, const CFGNode*> block_to_node_map;
    for (const auto& node : cfg.get_nodes()) {
        block_to_node_map[node.block] = &node;
    }

    // 1. 初始化：所有块的 OUT 状态为空
    for (const auto& block : func.blocks) {
        m_out_states[&block] = {};
    }

    // 2. 不动点迭代，直到没有 OUT 状态发生变化
    bool changed = true;
    while (changed) {
        changed = false;

        // 遍历所有基本块 (正向)
        for (const auto& block : func.blocks) {

            // 2.1 计算 IN 状态：合并所有前驱的 OUT 状态
            ConstState new_in_state;

            // 【修复】通过映射找到当前块对应的CFGNode，然后获取前驱
            const CFGNode* current_node = block_to_node_map.at(&block);
            const auto& predecessors = current_node->preds;

            if (!predecessors.empty()) {
                // 以第一个前驱的 OUT 状态为基础
                new_in_state = m_out_states.at(predecessors[0]->block);

                // 与其他前驱的 OUT 状态求 "交集"
                for (size_t i = 1; i < predecessors.size(); ++i) {
                    const BasicBlock* pred_block = predecessors[i]->block;
                    const ConstState& pred_out_state = m_out_states.at(pred_block);

                    std::vector<std::string> vars_to_remove;
                    for (auto const& [var, val] : new_in_state) {
                        if (pred_out_state.find(var) == pred_out_state.end() || pred_out_state.at(var) != val) {
                            vars_to_remove.push_back(var);
                        }
                    }
                    for (const auto& var : vars_to_remove) {
                        new_in_state.erase(var);
                    }
                }
            }
            m_in_states[&block] = new_in_state;

            // 2.2 传递函数：根据 IN 状态和块内指令，计算新的 OUT 状态
            ConstState current_state = m_in_states.at(&block);
            for (const auto& instr : block.instructions) {
                if (instr.result.kind != Operand::NONE) {
                    std::string dest_id = get_operand_id(instr.result);
                    if (!dest_id.empty()) {
                        current_state.erase(dest_id);

                        if (instr.opcode == Instruction::ASSIGN && instr.arg1.kind == Operand::CONST) {
                            current_state[dest_id] = instr.arg1.value;
                        }
                        else {
                            Operand const_op1 = instr.arg1;
                            Operand const_op2 = instr.arg2;

                            if (const_op1.kind != Operand::CONST) {
                                std::string op1_id = get_operand_id(const_op1);
                                if (!op1_id.empty() && current_state.count(op1_id)) {
                                    const_op1 = { Operand::CONST, "", current_state.at(op1_id), 0 };
                                }
                            }
                            if (const_op2.kind != Operand::CONST) {
                                std::string op2_id = get_operand_id(const_op2);
                                if (!op2_id.empty() && current_state.count(op2_id)) {
                                    const_op2 = { Operand::CONST, "", current_state.at(op2_id), 0 };
                                }
                            }

                            if (const_op1.kind == Operand::CONST && const_op2.kind == Operand::CONST) {
                                int val1 = const_op1.value;
                                int val2 = const_op2.value;
                                int result;
                                bool folded = true;
                                switch (instr.opcode) {
                                case Instruction::ADD: result = val1 + val2; break;
                                case Instruction::SUB: result = val1 - val2; break;
                                case Instruction::MUL: result = val1 * val2; break;
                                case Instruction::DIV: if (val2 != 0) result = val1 / val2; else folded = false; break;
                                default: folded = false; break;
                                }
                                if (folded) {
                                    current_state[dest_id] = result;
                                }
                            }
                        }
                    }
                }
            }

            // 2.3 检查 OUT 状态是否改变
            if (m_out_states.at(&block) != current_state) {
                m_out_states.at(&block) = current_state;
                changed = true;
            }
        }
    }
}
