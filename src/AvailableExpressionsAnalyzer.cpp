#include "AvailableExpressionsAnalyzer.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <algorithm> // For std::set_intersection
#include <iterator>  // For std::inserter
#include <unordered_map>

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
    // ע�⣺Ϊ���� a+b �� b+a ����Ϊ��ͬ���ʽ�����ԶԲ���������
    std::string op1_id = get_operand_id(instr.arg1);
    std::string op2_id = get_operand_id(instr.arg2);
    if (op1_id > op2_id) std::swap(op1_id, op2_id);

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

    // 1. ��ʼ�������п�� OUT ״̬Ϊ��
    for (const auto& block : func.blocks) {
        m_out_states[&block] = {};
    }

    // 2. ���������
    bool changed = true;
    while (changed) {
        changed = false;

        for (const auto& block : func.blocks) {
            // 2.1 ���� IN ״̬: IN[B] = Intersect(OUT[P]) for all P in pred(B)
            AvailableExprsMap new_in_state;
            const CFGNode* current_node = block_to_node_map.at(&block);

            if (!current_node->preds.empty()) {
                // �Ե�һ��ǰ����OUTΪ����
                new_in_state = m_out_states.at(current_node->preds[0]->block);

                // ������ǰ���󽻼�
                for (size_t i = 1; i < current_node->preds.size(); ++i) {
                    const auto& pred_out = m_out_states.at(current_node->preds[i]->block);
                    std::vector<std::string> keys_to_remove;
                    // ������ǰ�����е����б��ʽ
                    for (auto const& [expr, op] : new_in_state) {
                        // �����һ��ǰ��û��������ʽ�����߽����������ͬ����˱��ʽ�ڽ���㲻����
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
                // a. KILL set: �κζԱ����ĸ�ֵ����ɱ����صı��ʽ
                if (is_variable_or_temp(instr.result)) {
                    std::string dest_id = get_operand_id(instr.result);
                    std::vector<std::string> exprs_to_kill;
                    for (auto const& [expr, op] : current_out) {
                        // �������ֵ�ı�����ĳ�����ñ��ʽ�Ľ�������������ñ��ʽʧЧ
                        if (get_operand_id(op) == dest_id || expr.find(dest_id) != std::string::npos) {
                            exprs_to_kill.push_back(expr);
                        }
                    }
                    for (const auto& expr : exprs_to_kill) {
                        current_out.erase(expr);
                    }
                }

                // b. GEN set: ������һ���µı��ʽ
                if (instr.opcode >= Instruction::ADD && instr.opcode <= Instruction::GE) {
                    current_out[get_expr_id(instr)] = instr.result;
                }
            }

            // 2.3 ��� OUT ״̬�Ƿ�ı�
            if (m_out_states.at(&block) != current_out) {
                m_out_states.at(&block) = current_out;
                changed = true;
            }
        }
    }
}

