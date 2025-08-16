#include "AvailableExpressionsAnalyzer.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <algorithm> // For std::set_intersection
#include <iterator>  // For std::inserter
#include <unordered_map>

// --- 辅助函数 (可以从Optimizer.cpp复制或放在一个公共头文件中) ---

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

// 用于为表达式创建一个唯一的字符串ID
static std::string get_expr_id(const Instruction& instr) {
    std::stringstream ss;
    // 注意：为了让 a+b 和 b+a 被视为相同表达式，可以对操作数排序
    std::string op1_id = get_operand_id(instr.arg1);
    std::string op2_id = get_operand_id(instr.arg2);
    if (op1_id > op2_id) std::swap(op1_id, op2_id);

    ss << instr.opcode << " " << op1_id << " " << op2_id;
    return ss.str();
}

// --- AvailableExpressionsAnalyzer 实现 ---

void AvailableExpressionsAnalyzer::run(FunctionIR& func, ControlFlowGraph& cfg) {
    m_in_states.clear();
    m_out_states.clear();

    std::unordered_map<const BasicBlock*, const CFGNode*> block_to_node_map;
    for (const auto& node : cfg.get_nodes()) {
        block_to_node_map[node.block] = &node;
    }

    // 1. 初始化：所有块的 OUT 状态为空
    for (const auto& block : func.blocks) {
        m_out_states[&block] = {};
    }

    // 2. 不动点迭代
    bool changed = true;
    while (changed) {
        changed = false;

        for (const auto& block : func.blocks) {
            // 2.1 计算 IN 状态: IN[B] = Intersect(OUT[P]) for all P in pred(B)
            AvailableExprsMap new_in_state;
            const CFGNode* current_node = block_to_node_map.at(&block);

            if (!current_node->preds.empty()) {
                // 以第一个前驱的OUT为基础
                new_in_state = m_out_states.at(current_node->preds[0]->block);

                // 与其他前驱求交集
                for (size_t i = 1; i < current_node->preds.size(); ++i) {
                    const auto& pred_out = m_out_states.at(current_node->preds[i]->block);
                    std::vector<std::string> keys_to_remove;
                    // 遍历当前交集中的所有表达式
                    for (auto const& [expr, op] : new_in_state) {
                        // 如果另一个前驱没有这个表达式，或者结果操作数不同，则此表达式在交汇点不可用
                        if (pred_out.find(expr) == pred_out.end() || get_operand_id(pred_out.at(expr)) != get_operand_id(op)) {
                            keys_to_remove.push_back(expr);
                        }
                    }
                    for (const auto& key : keys_to_remove) {
                        new_in_state.erase(key);
                    }
                }
            }
            m_in_states[&block] = new_in_state;

            // 2.2 传递函数: OUT[B] = GEN[B] U (IN[B] - KILL[B])
            AvailableExprsMap current_out = m_in_states.at(&block);

            for (const auto& instr : block.instructions) {
                // a. KILL set: 任何对变量的赋值都会杀死相关的表达式
                if (is_variable_or_temp(instr.result)) {
                    std::string dest_id = get_operand_id(instr.result);
                    std::vector<std::string> exprs_to_kill;
                    for (auto const& [expr, op] : current_out) {
                        // 如果被赋值的变量是某个可用表达式的结果或操作数，则该表达式失效
                        if (get_operand_id(op) == dest_id || expr.find(dest_id) != std::string::npos) {
                            exprs_to_kill.push_back(expr);
                        }
                    }
                    for (const auto& expr : exprs_to_kill) {
                        current_out.erase(expr);
                    }
                }

                // b. GEN set: 计算了一个新的表达式
                if (instr.opcode >= Instruction::ADD && instr.opcode <= Instruction::GE) {
                    current_out[get_expr_id(instr)] = instr.result;
                }
            }

            // 2.3 检查 OUT 状态是否改变
            if (m_out_states.at(&block) != current_out) {
                m_out_states.at(&block) = current_out;
                changed = true;
            }
        }
    }
}

