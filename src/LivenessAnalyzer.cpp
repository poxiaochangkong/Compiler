#include "LivenessAnalyzer.hpp"
#include "ControlFlowGraph.hpp"
#include <vector>
#include <string>
#include <set>
#include <iterator>
#include <algorithm> // For std::set_difference, std::set_union

// �������������ڻ�ȡ������ID
static std::string get_operand_id(const Operand& op) {
    if (op.kind == Operand::VAR) return op.name;
    if (op.kind == Operand::TEMP) return "t" + std::to_string(op.id);
    return "";
}

// �������������������Ƿ��Ǳ�������ʱ����
static bool is_variable_or_temp(const Operand& op) {
    return op.kind == Operand::VAR || op.kind == Operand::TEMP;
}

void LivenessAnalyzer::run(ControlFlowGraph& cfg) {
    auto& nodes = const_cast<std::vector<CFGNode>&>(cfg.get_nodes());

    // 1. ����ÿ��������� def �� use ����
    for (auto& node : nodes) {
        std::set<std::string> defined_in_block;
        for (const auto& instr : node.block->instructions) {
            // ���� use ����
            if (is_variable_or_temp(instr.arg1) && defined_in_block.find(get_operand_id(instr.arg1)) == defined_in_block.end()) {
                node.use.insert(get_operand_id(instr.arg1));
            }
            if (is_variable_or_temp(instr.arg2) && defined_in_block.find(get_operand_id(instr.arg2)) == defined_in_block.end()) {
                node.use.insert(get_operand_id(instr.arg2));
            }

            // ���� def ����
            if (is_variable_or_temp(instr.result)) {
                node.def.insert(get_operand_id(instr.result));
                defined_in_block.insert(get_operand_id(instr.result));
            }
        }
    }

    // 2. �������� live_in �� live_out ���ϣ�ֱ���ȶ�
    bool changed = true;
    while (changed) {
        changed = false;
        // ���ǴӺ���ǰ���������飬�������Ը�������
        for (auto it = nodes.rbegin(); it != nodes.rend(); ++it) {
            auto& node = *it;

            // a. ���� live_out: live_out[B] = U (live_in[S]) for S in succ(B)
            std::set<std::string> new_live_out;
            for (const auto* succ : node.succs) {
                for (const auto& var : succ->live_in) {
                    new_live_out.insert(var);
                }
            }

            // b. ���� live_in: live_in[B] = use[B] U (live_out[B] - def[B])
            std::set<std::string> diff;
            std::set_difference(new_live_out.begin(), new_live_out.end(),
                node.def.begin(), node.def.end(),
                std::inserter(diff, diff.begin()));

            std::set<std::string> new_live_in;
            std::set_union(node.use.begin(), node.use.end(),
                diff.begin(), diff.end(),
                std::inserter(new_live_in, new_live_in.begin()));

            // c. ��鼯���Ƿ����仯
            if (new_live_in != node.live_in || new_live_out != node.live_out) {
                changed = true;
                node.live_in = new_live_in;
                node.live_out = new_live_out;
            }
        }
    }
}
