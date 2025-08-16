#include "ControlFlowGraph.hpp"
#include "LivenessAnalyzer.hpp"
#include <iostream>

extern std::string operand_to_string(const Operand& op);

ControlFlowGraph::ControlFlowGraph(const FunctionIR& func) : m_func(func) {
    build();
}

// ������� build ����
void ControlFlowGraph::build() {
    if (m_func.blocks.empty()) return;

    // ��һ������ʼ���ڵ�ͱ�ǩ���ڵ��ӳ�� (�ⲿ���߼�����ȷ��)
    m_nodes.resize(m_func.blocks.size());
    for (size_t i = 0; i < m_func.blocks.size(); ++i) {
        m_nodes[i].block = &m_func.blocks[i];
        m_nodes[i].id = i;
        m_label_to_node[m_func.blocks[i].label] = &m_nodes[i];
    }

    // �ڶ������������нڵ㣬����ǰ��̹�ϵ
    for (size_t i = 0; i < m_nodes.size(); ++i) {
        CFGNode* current_node = &m_nodes[i];
        const auto& instructions = current_node->block->instructions;

        // Ĭ������£�һ����ִ����ᡰ�ᴩ������һ����
        bool falls_through = true;

        if (!instructions.empty()) {
            const Instruction& last_instr = instructions.back();

            // ���1: ��������ת
            if (last_instr.opcode == Instruction::JUMP) {
                CFGNode* target_node = m_label_to_node.at(last_instr.arg1.name);
                current_node->succs.push_back(target_node);
                target_node->preds.push_back(current_node);
                // ��������ת����ֹ�ᴩ
                falls_through = false;
            }
            // ���2: ������ת
            else if (last_instr.opcode == Instruction::JUMP_IF_ZERO || last_instr.opcode == Instruction::JUMP_IF_NZERO) {
                // ������ת��һ����ȷ��Ŀ����
                CFGNode* target_node = m_label_to_node.at(last_instr.arg2.name);
                current_node->succs.push_back(target_node);
                target_node->preds.push_back(current_node);
                // ͬʱ����Ҳ����ᴩ����һ��ָ���Ϊif-else����һ��֧��
                // falls_through ����Ϊ true
            }
            // ���3: ����ָ��
            else if (last_instr.opcode == Instruction::RET) {
                // ����ָ�����ֹ��ǰ�Ŀ���������˲���ᴩ
                falls_through = false;
            }
            // ������������ָ�ADD, MUL, ASSIGN�ȣ������ǲ���ı������
            // falls_through ����Ϊ true
        }

        // �����ǰ������ᴩ�����������Ǻ����е����һ����
        if (falls_through && (i + 1 < m_nodes.size())) {
            // ���һ������һ����ı�
            CFGNode* next_node = &m_nodes[i + 1];
            current_node->succs.push_back(next_node);
            next_node->preds.push_back(current_node);
        }
    }
    //this->print_dot(m_func.name);
}

// --- �����������ֲ��� ---
void ControlFlowGraph::run_liveness_analysis() {
    LivenessAnalyzer analyzer;
    analyzer.run(*this);
}

void ControlFlowGraph::print_dot(const std::string& func_name) const {
    std::cout << "digraph " << func_name << " {" << std::endl;
    for (const auto& node : m_nodes) {
        std::cout << "  \"" << node.block->label << "\";" << std::endl;
        for (const auto& instr : node.block->instructions) {
            std::cout << "  ";
            switch (instr.opcode) {
                // --- �������� ---
            case Instruction::ADD:
                std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " + " << operand_to_string(instr.arg2);
                break;
            case Instruction::SUB:
                std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " - " << operand_to_string(instr.arg2);
                break;
            case Instruction::MUL:
                std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " * " << operand_to_string(instr.arg2);
                break;
            case Instruction::DIV:
                std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " / " << operand_to_string(instr.arg2);
                break;
            case Instruction::MOD:
                std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " % " << operand_to_string(instr.arg2);
                break;

                // --- �߼����ϵ���� ---
            case Instruction::NOT:
                std::cout << operand_to_string(instr.result) << " = NOT " << operand_to_string(instr.arg1);
                break;
            case Instruction::EQ:
                std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " == " << operand_to_string(instr.arg2);
                break;
            case Instruction::NEQ:
                std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " != " << operand_to_string(instr.arg2);
                break;
            case Instruction::LT:
                std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " < " << operand_to_string(instr.arg2);
                break;
            case Instruction::GT:
                std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " > " << operand_to_string(instr.arg2);
                break;
            case Instruction::LE:
                std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " <= " << operand_to_string(instr.arg2);
                break;
            case Instruction::GE:
                std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1) << " >= " << operand_to_string(instr.arg2);
                break;

                // --- ��ֵ ---
            case Instruction::ASSIGN:
                std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1);
                break;


                // --- ���� ---
            case Instruction::PARAM: // <-- ����
                std::cout << "PARAM " << operand_to_string(instr.arg1);
                break;
            case Instruction::CALL:
                // ����Ҳ��ӡ��������
                std::cout << operand_to_string(instr.result) << " = CALL " << operand_to_string(instr.arg1) << ", " << operand_to_string(instr.arg2);
                break;
            case Instruction::RET:
                std::cout << "RET " << (instr.arg1.kind != Operand::NONE ? operand_to_string(instr.arg1) : "");
                break;

                // --- ��֧���ǩ ---
            case Instruction::JUMP:
                std::cout << "JUMP " << operand_to_string(instr.arg1);
                break;
            case Instruction::JUMP_IF_ZERO:
                std::cout << "IF " << operand_to_string(instr.arg1) << " == 0 JUMP " << operand_to_string(instr.arg2);
                break;
            case Instruction::JUMP_IF_NZERO:
                std::cout << "IF " << operand_to_string(instr.arg1) << " != 0 JUMP " << operand_to_string(instr.arg2);
                break;
            case Instruction::LABEL:
                // ��ǩ������ǿ�����֣�ͨ������Ҫ��Ϊָ���ӡ
                continue; // ������ӡ
            }
            std::cout << std::endl;
        }
        for (const auto* succ : node.succs) {
            std::cout << "  \"" << node.block->label << "\" -> \"" << succ->block->label << "\";" << std::endl;
        }
        
    }
    std::cout << "}" << std::endl;
}
