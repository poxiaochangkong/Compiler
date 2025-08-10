#include "CodeGenerator.hpp"
#include "SpillEverythingAllocator.hpp" // 包含具体实现
#include <iostream>
#include <stdexcept>

// 在构造函数中选择策略
CodeGenerator::CodeGenerator() {
    // 未来想换策略时，只需修改这一行！
    m_allocator = std::make_unique<SpillEverythingAllocator>();
}

// generate_function 现在只负责流程控制
void CodeGenerator::generate_function(const FunctionIR& func) {
    // 1. 准备阶段
    m_allocator->prepare(func);

    // 2. 生成序言
    m_output << m_allocator->getPrologue();

    // 3. 逐条生成指令
    for (const auto& bb : func.blocks) {
        if (!bb.label.empty()) {
            m_output << bb.label << ":\n";
        }
        for (const auto& instr : bb.instructions) {
            generate_instruction(instr);
        }
    }
}

// generate_instruction 现在调用 m_allocator 来处理访存
void CodeGenerator::generate_instruction(const Instruction& instr) {
    switch (instr.opcode) {
    case Instruction::ADD: case Instruction::SUB: case Instruction::MUL: case Instruction::DIV: case Instruction::MOD: {
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << m_allocator->loadOperand(instr.arg2, "t1");
        std::string op_str;
        if (instr.opcode == Instruction::ADD) op_str = "add";
        if (instr.opcode == Instruction::SUB) op_str = "sub";
        if (instr.opcode == Instruction::MUL) op_str = "mul";
        if (instr.opcode == Instruction::DIV) op_str = "div";
        if (instr.opcode == Instruction::MOD) op_str = "rem";
        m_output << "  " << op_str << " t2, t0, t1\n";
        m_output << m_allocator->storeOperand(instr.result, "t2");
        break;
    }
    case Instruction::NOT: { // 逻辑非
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << "  seqz t0, t0\n"; // 如果 t0 为 0，则 t0=1；否则 t0=0
        m_output << m_allocator->storeOperand(instr.result, "t0");
        break;
    }

    case Instruction::EQ: { // 等于 ==
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << m_allocator->loadOperand(instr.arg2, "t1");
        m_output << "  sub t2, t0, t1\n";
        m_output << "  seqz t2, t2\n"; // 如果 t2 (差值) 为 0，则 t2=1
        m_output << m_allocator->storeOperand(instr.result, "t2");
        break;
    }

    case Instruction::NEQ: { // 不等于 !=
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << m_allocator->loadOperand(instr.arg2, "t1");
        m_output << "  sub t2, t0, t1\n";
        m_output << "  snez t2, t2\n"; // 如果 t2 (差值) 不为 0，则 t2=1
        m_output << m_allocator->storeOperand(instr.result, "t2");
        break;
    }

    case Instruction::LT: { // 小于 <
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << m_allocator->loadOperand(instr.arg2, "t1");
        m_output << "  slt t2, t0, t1\n"; // 如果 t0 < t1，则 t2=1
        m_output << m_allocator->storeOperand(instr.result, "t2");
        break;
    }

    case Instruction::GT: { // 大于 >
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << m_allocator->loadOperand(instr.arg2, "t1");
        m_output << "  sgt t2, t0, t1\n"; // 如果 t0 > t1，则 t2=1 (sgt是伪指令，等价于 slt t2, t1, t0)
        m_output << m_allocator->storeOperand(instr.result, "t2");
        break;
    }

    case Instruction::LE: { // 小于等于 <= (已有)
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << m_allocator->loadOperand(instr.arg2, "t1");
        m_output << "  sgt t2, t0, t1\n"; // t2 = (t0 > t1)
        m_output << "  xori t2, t2, 1\n"; // t2 = !t2
        m_output << m_allocator->storeOperand(instr.result, "t2");
        break;
    }

    case Instruction::GE: { // 大于等于 >=
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << m_allocator->loadOperand(instr.arg2, "t1");
        m_output << "  slt t2, t0, t1\n"; // t2 = (t0 < t1)
        m_output << "  xori t2, t2, 1\n"; // t2 = !t2
        m_output << m_allocator->storeOperand(instr.result, "t2");
        break;
    }

    case Instruction::JUMP_IF_ZERO: {
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << "  beqz t0, " << instr.arg2.name << "\n";
        break;
    }

    case Instruction::JUMP_IF_NZERO: { // JUMP_IF_NZERO (不为0则跳转)
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << "  bnez t0, " << instr.arg2.name << "\n";
        break;
    case Instruction::JUMP:
        m_output << "  j " << instr.arg1.name << "\n";
        break;
    case Instruction::ASSIGN:
        m_output << m_allocator->loadOperand(instr.arg1, "t0");
        m_output << m_allocator->storeOperand(instr.result, "t0");
        break;
    case Instruction::RET: {
        if (instr.arg1.kind != Operand::NONE) {
            m_output << m_allocator->loadOperand(instr.arg1, "a0");
        }
        // 直接获取尾声代码
        m_output << m_allocator->getEpilogue();
        break;
    }
    case Instruction::PARAM:
        if (m_param_idx < 8) {
            // 前8个参数通过寄存器 a0-a7 传递
            m_output << m_allocator->loadOperand(instr.arg1, "a" + std::to_string(m_param_idx));
        }
        else {
            // 后续参数通过栈传递
            // 1. 将参数值加载到临时寄存器 t0
            m_output << m_allocator->loadOperand(instr.arg1, "t0");
            // 2. 将 t0 的值存入栈中，偏移量根据参数序号计算
            m_output << "  sw t0, " << (m_param_idx - 8) * 4 << "(sp)\n";
        }
        m_param_idx++;
        break;
    case Instruction::CALL:
        m_param_idx = 0; // 重置参数计数器
        m_output << "  call " << instr.arg1.name << "\n";
        if (instr.result.kind != Operand::NONE) {
            // 将返回值 a0 存到结果变量的位置
            m_output << m_allocator->storeOperand(instr.result, "a0");
        }
        break;
    case Instruction::LABEL:
        // Label 不生成代码，由 generate_function 处理
        break;
    default:
        m_output << "  # Unhandled OpCode: " << instr.opcode << "\n";
        break;
    }
    }
}

//主入口函数，确保 main 函数最先生成
std::string CodeGenerator::generate(const ModuleIR& module) {
    m_output.str("");
    m_output.clear();
    m_output << ".text\n.globl main\n\n";

    // 1. 查找并优先生成 main 函数
    const FunctionIR* main_func = nullptr;
    for (const auto& func : module.functions) {
        if (func.name == "main") {
            main_func = &func;
            break;
        }
    }

    if (main_func) {
        m_param_idx = 0; // 为 main 函数重置参数计数器
        generate_function(*main_func);
        m_output << "\n";
    }
    else {
        // 对于一个独立的可执行程序来说，没有 main 函数是致命错误
        throw std::runtime_error("CodeGenerator Error: 'main' function not found in module.");
    }

    // 2. 生成其他所有非 main 函数
    for (const auto& func : module.functions) {
        if (func.name != "main") {
            m_param_idx = 0; // 每个函数开始前重置参数计数器
            generate_function(func);
            m_output << "\n";
        }
    }

    return m_output.str();
}