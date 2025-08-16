#include "AvailableExpressionsAnalyzer.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <algorithm> // For std::set_intersection
#include <iterator>  // For std::inserter
#include <unordered_map>
#include <unordered_set>

// --- �������� (���Դ�Optimizer.cpp���ƻ����һ������ͷ�ļ���) ---

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

// ����Ϊ���ʽ����һ��Ψһ���ַ���ID
static std::string get_expr_id(const Instruction& instr) {
    std::stringstream ss;
    std::string op1_id = get_operand_id(instr.arg1);
    std::string op2_id = get_operand_id(instr.arg2);
    // �����ɴ���
    if (instr.opcode == Instruction::ADD || instr.opcode == Instruction::MUL ||
        instr.opcode == Instruction::EQ || instr.opcode == Instruction::NEQ) {
        if (op1_id > op2_id) std::swap(op1_id, op2_id);
    }

    ss << instr.opcode << " " << op1_id << " " << op2_id;
    return ss.str();
}

// --- AvailableExpressionsAnalyzer ʵ�� ---

void AvailableExpressionsAnalyzer::run(FunctionIR& func, ControlFlowGraph& cfg) {
    m_in_states.clear();
    m_out_states.clear();

    std::unordered_map<const BasicBlock*, const CFGNode*> block_to_node_map;
    for (const auto& node : cfg.get_nodes()) {
        block_to_node_map[node.block] = &node;
    }

    for (const auto& block : func.blocks) {
        m_out_states[&block] = {};
    }

    bool changed = true;
    while (changed) {
        changed = false;

        for (const auto& block : func.blocks) {
            // 2.1 ���� IN ״̬ (�󽻼�)
            AvailableExprsMap new_in_state;
            const CFGNode* current_node = block_to_node_map.at(&block);

            if (!current_node->preds.empty()) {
                new_in_state = m_out_states.at(current_node->preds[0]->block);
                for (size_t i = 1; i < current_node->preds.size(); ++i) {
                    const auto& pred_out = m_out_states.at(current_node->preds[i]->block);
                    std::vector<std::string> keys_to_remove;
                    for (auto const& [expr, op] : new_in_state) {
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

            // 2.2 ���ݺ���: OUT[B] = GEN[B] U (IN[B] - KILL[B])
            AvailableExprsMap current_out = m_in_states.at(&block);

            for (const auto& instr : block.instructions) {
                // a. KILL set: ʹ�ø���Ч������ȷ�ķ�ʽ
                if (is_variable_or_temp(instr.result)) {
                    std::string dest_id = get_operand_id(instr.result);
                    std::vector<std::string> exprs_to_kill;

                    for (auto const& [expr, result_op] : current_out) {
                        // 1. ������ʽ�Ľ������д��ɱ����
                        if (get_operand_id(result_op) == dest_id) {
                            exprs_to_kill.push_back(expr);
                            continue;
                        }

                        // 2. ���ؼ��޸���������ʽ�Ĳ���������д��ɱ����
                        // ����ͨ�������ַ�������ȷƥ�������
                        std::stringstream ss(expr);
                        std::string opcode_str, op1_str, op2_str;
                        ss >> opcode_str >> op1_str >> op2_str; // �� "opcode op1 op2" ��ʽ����

                        if (op1_str == dest_id || op2_str == dest_id) {
                            exprs_to_kill.push_back(expr);
                        }
                    }
                    // ʹ����ʱ���ϱ����ظ�ɾ��
                    std::unordered_set<std::string> unique_exprs_to_kill(exprs_to_kill.begin(), exprs_to_kill.end());
                    for (const auto& expr : unique_exprs_to_kill) {
                        current_out.erase(expr);
                    }
                }

                // b. GEN set
                if (instr.opcode >= Instruction::ADD && instr.opcode <= Instruction::GE) {
                    if (is_variable_or_temp(instr.arg1) || is_variable_or_temp(instr.arg2)) {
                        current_out[get_expr_id(instr)] = instr.result;
                    }
                }
            }

            if (m_out_states.at(&block) != current_out) {
                m_out_states.at(&block) = current_out;
                changed = true;
            }
        }
    }
}

