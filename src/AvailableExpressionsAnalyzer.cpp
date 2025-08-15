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

    // 1. 初始化：入口块的IN为空，其他所有块的IN为全集U (所有表达式)
    // 这样可以正确地进行交集操作
    AvailableExprsSet universal_set;
    std::map<std::string, std::vector<std::string>> expr_to_operands;

    // 首先，收集函数中所有的表达式
    for (const auto& block : func.blocks) {
        for (const auto& instr : block.instructions) {
            if (instr.opcode >= Instruction::ADD && instr.opcode <= Instruction::GE) {
                std::string expr = get_expr_id(instr);
                universal_set.insert(expr);
                expr_to_operands[expr] = { get_operand_id(instr.arg1), get_operand_id(instr.arg2) };
            }
        }
    }

    for (const auto& block : func.blocks) {
        m_out_states[&block] = universal_set;
    }
    // 入口块的OUT状态是空的，因为它没有前驱
    if (!func.blocks.empty()) {
        m_out_states[&func.blocks.front()] = {};
    }

    // 2. 不动点迭代
    bool changed = true;
    while (changed) {
        changed = false;

        for (const auto& block : func.blocks) {
            // 2.1 计算 IN 状态: IN[B] = Intersect(OUT[P]) for all P in pred(B)
            AvailableExprsSet new_in_state = universal_set; // 从全集开始
            if (&block == &func.blocks.front()) {
                new_in_state.clear(); // 入口块的IN为空
            }

            const CFGNode* current_node = block_to_node_map.at(&block);
            if (!current_node->preds.empty()) {
                // 以第一个前驱的OUT为基础
                new_in_state = m_out_states.at(current_node->preds[0]->block);
                // 与其他前驱求交集
                for (size_t i = 1; i < current_node->preds.size(); ++i) {
                    const auto& pred_out = m_out_states.at(current_node->preds[i]->block);
                    AvailableExprsSet temp_intersect;
                    std::set_intersection(new_in_state.begin(), new_in_state.end(),
                        pred_out.begin(), pred_out.end(),
                        std::inserter(temp_intersect, temp_intersect.begin()));
                    new_in_state = temp_intersect;
                }
            }
            m_in_states[&block] = new_in_state;

            // 2.2 传递函数: OUT[B] = GEN[B] U (IN[B] - KILL[B])
            AvailableExprsSet current_out = m_in_states.at(&block);

            for (const auto& instr : block.instructions) {
                // a. KILL set: 任何对变量的赋值都会杀死包含该变量的表达式
                if (is_variable_or_temp(instr.result)) {
                    std::string dest_id = get_operand_id(instr.result);
                    std::vector<std::string> exprs_to_kill;
                    for (const auto& expr : current_out) {
                        if (expr_to_operands[expr][0] == dest_id || expr_to_operands[expr][1] == dest_id) {
                            exprs_to_kill.push_back(expr);
                        }
                    }
                    for (const auto& expr : exprs_to_kill) {
                        current_out.erase(expr);
                    }
                }

                // b. GEN set: 计算了一个新的表达式
                if (instr.opcode >= Instruction::ADD && instr.opcode <= Instruction::GE) {
                    current_out.insert(get_expr_id(instr));
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
