#include "ControlFlowGraph.hpp"
#include "LivenessAnalyzer.hpp" // <-- 包含新的分析器头文件
#include <iostream>

ControlFlowGraph::ControlFlowGraph(const FunctionIR& func) : m_func(func) {
    build();
}

void ControlFlowGraph::build() {
    if (m_func.blocks.empty()) return;

    m_nodes.resize(m_func.blocks.size());
    for (size_t i = 0; i < m_func.blocks.size(); ++i) {
        m_nodes[i].block = &m_func.blocks[i];
        m_nodes[i].id = i;
        m_label_to_node[m_func.blocks[i].label] = &m_nodes[i];
    }

    for (size_t i = 0; i < m_nodes.size(); ++i) {
        CFGNode* current_node = &m_nodes[i];
        const auto& instructions = current_node->block->instructions;

        bool fall_through = true;
        if (!instructions.empty()) {
            const Instruction& last_instr = instructions.back();

            if (last_instr.opcode == Instruction::JUMP) {
                CFGNode* target_node = m_label_to_node.at(last_instr.arg1.name);
                current_node->succs.push_back(target_node);
                target_node->preds.push_back(current_node);
                fall_through = false;
            }
            else if (last_instr.opcode == Instruction::JUMP_IF_ZERO || last_instr.opcode == Instruction::JUMP_IF_NZERO) {
                CFGNode* target_node = m_label_to_node.at(last_instr.arg2.name);
                current_node->succs.push_back(target_node);
                target_node->preds.push_back(current_node);
                fall_through = true;
            }
            else if (last_instr.opcode == Instruction::RET) {
                fall_through = false;
            }
            else {
                std::cerr << "block has no terminal, label:{"<<current_node->block->label<<std::endl;
            }
            //if(instructions.)
        }

        if (fall_through && (i + 1 < m_nodes.size())) {
            CFGNode* next_node = &m_nodes[i + 1];
            current_node->succs.push_back(next_node);
            next_node->preds.push_back(current_node);
        }
    }
}

// --- 新增：实现分析接口 ---
void ControlFlowGraph::run_liveness_analysis() {
    LivenessAnalyzer analyzer;
    analyzer.run(*this);
}

void ControlFlowGraph::print_dot(const std::string& func_name) const {
    std::cout << "digraph " << func_name << " {" << std::endl;
    for (const auto& node : m_nodes) {
        std::cout << "  \"" << node.block->label << "\";" << std::endl;
        for (const auto* succ : node.succs) {
            std::cout << "  \"" << node.block->label << "\" -> \"" << succ->block->label << "\";" << std::endl;
        }
    }
    std::cout << "}" << std::endl;
}
