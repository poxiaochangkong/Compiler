#include "ControlFlowGraph.hpp"
#include "LivenessAnalyzer.hpp"
#include <iostream>

extern std::string operand_to_string(const Operand& op);

ControlFlowGraph::ControlFlowGraph(const FunctionIR& func) : m_func(func) {
    build();
}

// 修正后的 build 函数
void ControlFlowGraph::build() {
    if (m_func.blocks.empty()) return;

    // 第一步：初始化节点和标签到节点的映射 (这部分逻辑是正确的)
    m_nodes.resize(m_func.blocks.size());
    for (size_t i = 0; i < m_func.blocks.size(); ++i) {
        m_nodes[i].block = &m_func.blocks[i];
        m_nodes[i].id = i;
        m_label_to_node[m_func.blocks[i].label] = &m_nodes[i];
    }

    // 第二步：遍历所有节点，建立前后继关系
    for (size_t i = 0; i < m_nodes.size(); ++i) {
        CFGNode* current_node = &m_nodes[i];
        const auto& instructions = current_node->block->instructions;

        // 默认情况下，一个块执行完会“贯穿”到下一个块
        bool falls_through = true;

        if (!instructions.empty()) {
            const Instruction& last_instr = instructions.back();

            // 情况1: 无条件跳转
            if (last_instr.opcode == Instruction::JUMP) {
                CFGNode* target_node = m_label_to_node.at(last_instr.arg1.name);
                current_node->succs.push_back(target_node);
                target_node->preds.push_back(current_node);
                // 无条件跳转会阻止贯穿
                falls_through = false;
            }
            // 情况2: 条件跳转
            else if (last_instr.opcode == Instruction::JUMP_IF_ZERO || last_instr.opcode == Instruction::JUMP_IF_NZERO) {
                // 条件跳转有一个明确的目标后继
                CFGNode* target_node = m_label_to_node.at(last_instr.arg2.name);
                current_node->succs.push_back(target_node);
                target_node->preds.push_back(current_node);
                // 同时，它也允许贯穿到下一条指令（作为if-else的另一分支）
                // falls_through 保持为 true
            }
            // 情况3: 返回指令
            else if (last_instr.opcode == Instruction::RET) {
                // 返回指令会终止当前的控制流，因此不会贯穿
                falls_through = false;
            }
            // 对于所有其他指令（ADD, MUL, ASSIGN等），它们不会改变控制流
            // falls_through 保持为 true
        }

        // 如果当前块允许贯穿，并且它不是函数中的最后一个块
        if (falls_through && (i + 1 < m_nodes.size())) {
            // 添加一条到下一个块的边
            CFGNode* next_node = &m_nodes[i + 1];
            current_node->succs.push_back(next_node);
            next_node->preds.push_back(current_node);
        }
    }
    //this->print_dot(m_func.name);
}

// --- 其他函数保持不变 ---
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
                // --- 算术运算 ---
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

                // --- 逻辑与关系运算 ---
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

                // --- 赋值 ---
            case Instruction::ASSIGN:
                std::cout << operand_to_string(instr.result) << " = " << operand_to_string(instr.arg1);
                break;


                // --- 函数 ---
            case Instruction::PARAM: // <-- 新增
                std::cout << "PARAM " << operand_to_string(instr.arg1);
                break;
            case Instruction::CALL:
                // 现在也打印参数数量
                std::cout << operand_to_string(instr.result) << " = CALL " << operand_to_string(instr.arg1) << ", " << operand_to_string(instr.arg2);
                break;
            case Instruction::RET:
                std::cout << "RET " << (instr.arg1.kind != Operand::NONE ? operand_to_string(instr.arg1) : "");
                break;

                // --- 分支与标签 ---
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
                // 标签本身就是块的名字，通常不需要作为指令打印
                continue; // 跳过打印
            }
            std::cout << std::endl;
        }
        for (const auto* succ : node.succs) {
            std::cout << "  \"" << node.block->label << "\" -> \"" << succ->block->label << "\";" << std::endl;
        }
        
    }
    std::cout << "}" << std::endl;
}
