#include "IRGenerator.hpp"
#include <stdexcept> // 用于 std::runtime_error
#include <string>    // 用于 std::to_string

ModuleIR IRGenerator::generate(Program* root) {
    if (root) {
        root->accept(this);
    }
    return m_module;
}

// --- 辅助函数实现 ---

Operand IRGenerator::new_temp() {
    Operand op;
    op.kind = Operand::TEMP;
    op.id = temp_counter++;
    return op;
}

Operand IRGenerator::new_label_op() {
    Operand op;
    op.kind = Operand::LABEL;
    op.id = label_counter++;
    return op;
}

BasicBlock* IRGenerator::create_block(const std::string& prefix) {
    BasicBlock* bb = new BasicBlock();
    Operand label_op = new_label_op();
    bb->label = prefix + std::to_string(label_op.id);
    return bb;
}

void IRGenerator::add_block(BasicBlock* bb) {
    if (current_func) {
        current_func->blocks.push_back(*bb);
        delete bb; // The block is copied into the vector, so we can delete the temporary.
        current_block = &current_func->blocks.back();
    }
}

Instruction::OpCode IRGenerator::map_bin_op(BinOp op) {
    switch (op) {
    case BinOp::Add: return Instruction::ADD;
    case BinOp::Sub: return Instruction::SUB;
    case BinOp::Mul: return Instruction::MUL;
    case BinOp::Div: return Instruction::DIV;
    case BinOp::Mod: return Instruction::MOD;
    case BinOp::Eq:  return Instruction::EQ;
    case BinOp::Neq: return Instruction::NEQ;
    case BinOp::Lt:  return Instruction::LT;
    case BinOp::Gt:  return Instruction::GT;
    case BinOp::Le:  return Instruction::LE;
    case BinOp::Ge:  return Instruction::GE;
    default:
        // LAnd 和 LOr 通过短路求值处理，不直接映射
        throw std::runtime_error("Unhandled binary operator for direct mapping");
    }
}

// --- Visitor 方法实现 ---

void IRGenerator::visit(Program* node) {
    for (auto* func : node->funcs) {
        func->accept(this);
    }
}

void IRGenerator::visit(FuncDef* node) {
    m_module.functions.emplace_back();
    current_func = &m_module.functions.back();
    current_func->name = node->name;

    // 每个函数都有一个入口块
    BasicBlock* entry_block = create_block(".entry_");
    add_block(entry_block);

    // 为函数参数生成对应的 IR 操作数（虽然在这个阶段不直接生成指令）
    for (auto* param : node->params) {
        param->accept(this);
    }

    node->body->accept(this);

    // 确保非 void 函数的所有路径都有返回（这是一个简化的检查）
    // 一个更完整的检查需要分析控制流图
    if (current_block && (current_block->instructions.empty() || current_block->instructions.back().opcode != Instruction::RET)) {
        if (node->ret == TypeKind::TY_VOID) {
            current_block->instructions.push_back({ Instruction::RET });
        }
    }
    current_func = nullptr;
}

void IRGenerator::visit(Param* node) {
    // 在 IR 生成阶段，参数主要用于函数签名，
    // 实际的参数传递在 visit(CallExpr*) 中处理。
    // 这里可以什么都不做。
}

void IRGenerator::visit(Block* node) {
    for (auto* stmt : node->stmts) {
        if (stmt) stmt->accept(this);
    }
}

void IRGenerator::visit(ExprStmt* node) {
    if (node->e) {
        node->e->accept(this);
    }
}

void IRGenerator::visit(AssignStmt* node) {
    // 1. 计算右侧表达式的值，结果在 m_result_op 中
    node->rhs->accept(this);
    Operand src_op = m_result_op;

    // 2. 创建代表左侧变量的操作数
    Operand dest_op;
    dest_op.kind = Operand::VAR;
    dest_op.name = node->name;

    // 3. 生成赋值指令
    current_block->instructions.push_back({ Instruction::ASSIGN, dest_op, src_op });
}

void IRGenerator::visit(DeclStmt* node) {
    // 变量声明，如果有初始化，就等同于一次赋值
    if (node->init) {
        node->init->accept(this);
        Operand src_op = m_result_op;

        Operand dest_op;
        dest_op.kind = Operand::VAR;
        dest_op.name = node->name;

        current_block->instructions.push_back({ Instruction::ASSIGN, dest_op, src_op });
    }
}

void IRGenerator::visit(ReturnStmt* node) {
    Instruction instr;
    instr.opcode = Instruction::RET;
    if (node->e) {
        node->e->accept(this);
        instr.arg1 = m_result_op;
    }
    current_block->instructions.push_back(instr);
}

void IRGenerator::visit(BreakStmt* node) {
    if (!break_labels.empty()) {
        current_block->instructions.push_back({ Instruction::JUMP, {}, break_labels.back() });
    }
}

void IRGenerator::visit(ContinueStmt* node) {
    if (!continue_labels.empty()) {
        current_block->instructions.push_back({ Instruction::JUMP, {}, continue_labels.back() });
    }
}

void IRGenerator::visit(IfStmt* node) {
    BasicBlock* then_block = create_block();
    BasicBlock* merge_block = create_block();
    BasicBlock* else_block = node->elseS ? create_block() : merge_block;

    // 1. 计算条件表达式
    node->cond->accept(this);

    // 2. 生成条件跳转指令
    Operand false_dest_label;
    false_dest_label.kind = Operand::LABEL;
    false_dest_label.name = else_block->label;
    current_block->instructions.push_back({ Instruction::JUMP_IF_ZERO, {}, m_result_op, false_dest_label });
    // 如果条件为真，则顺序执行到 then 块（需要先跳转）
    Operand true_dest_label;
    true_dest_label.kind = Operand::LABEL;
    true_dest_label.name = then_block->label;
    current_block->instructions.push_back({ Instruction::JUMP, {}, true_dest_label });


    // 3. 生成 Then 块
    add_block(then_block);
    node->thenS->accept(this);
    // Then 块结束后无条件跳转到 Merge 块
    Operand merge_dest_label;
    merge_dest_label.kind = Operand::LABEL;
    merge_dest_label.name = merge_block->label;
    current_block->instructions.push_back({ Instruction::JUMP, {}, merge_dest_label });

    // 4. 生成 Else 块 (如果存在)
    if (node->elseS) {
        add_block(else_block);
        node->elseS->accept(this);
        // Else 块结束后也无条件跳转到 Merge 块
        current_block->instructions.push_back({ Instruction::JUMP, {}, merge_dest_label });
    }

    // 5. 后续代码在 Merge 块中生成
    add_block(merge_block);
}

void IRGenerator::visit(WhileStmt* node) {
    BasicBlock* cond_block = create_block(".while_cond_");
    BasicBlock* body_block = create_block(".while_body_");
    BasicBlock* end_block = create_block(".while_end_");

    // 设置 break 和 continue 的跳转目标
    Operand end_label_op;
    end_label_op.kind = Operand::LABEL;
    end_label_op.name = end_block->label;
    break_labels.push_back(end_label_op);

    Operand cond_label_op;
    cond_label_op.kind = Operand::LABEL;
    cond_label_op.name = cond_block->label;
    continue_labels.push_back(cond_label_op);

    // 无条件跳转到条件判断块
    current_block->instructions.push_back({ Instruction::JUMP, {}, cond_label_op });

    // 1. 生成条件判断块
    add_block(cond_block);
    node->cond->accept(this);
    Operand body_label_op;
    body_label_op.kind = Operand::LABEL;
    body_label_op.name = body_block->label;
    current_block->instructions.push_back({ Instruction::JUMP_IF_NZERO, {}, m_result_op, body_label_op });
    // 如果条件为假，跳转到循环结束块
    current_block->instructions.push_back({ Instruction::JUMP, {}, end_label_op });

    // 2. 生成循环体块
    add_block(body_block);
    node->body->accept(this);
    // 循环体结束后，无条件跳回条件判断块
    current_block->instructions.push_back({ Instruction::JUMP, {}, cond_label_op });

    // 3. 后续代码在循环结束块中生成
    add_block(end_block);

    // 离开循环，弹出对应的标签
    break_labels.pop_back();
    continue_labels.pop_back();
}

void IRGenerator::visit(IntLiteral* node) {
    m_result_op.kind = Operand::CONST;
    m_result_op.value = node->value;
}

void IRGenerator::visit(VarExpr* node) {
    m_result_op.kind = Operand::VAR;
    m_result_op.name = node->name;
}

void IRGenerator::visit(BinaryExpr* node) {
    // --- 处理短路求值 ---
    if (node->op == BinOp::LAnd || node->op == BinOp::LOr) {
        Operand res = new_temp();
        BasicBlock* short_circuit_block = create_block();
        BasicBlock* end_block = create_block();

        Operand short_circuit_label;
        short_circuit_label.kind = Operand::LABEL;
        short_circuit_label.name = short_circuit_block->label;

        Operand end_label;
        end_label.kind = Operand::LABEL;
        end_label.name = end_block->label;

        node->lhs->accept(this);
        if (node->op == BinOp::LAnd) {
            // a && b: if a is 0, result is 0, jump to end.
            current_block->instructions.push_back({ Instruction::JUMP_IF_ZERO, {}, m_result_op, short_circuit_label });
        }
        else { // LOr
            // a || b: if a is not 0, result is 1, jump to end.
            current_block->instructions.push_back({ Instruction::JUMP_IF_NZERO, {}, m_result_op, short_circuit_label });
        }

        // 如果没有短路，则计算右侧表达式
        node->rhs->accept(this);
        // 将右侧表达式的结果（0或非0）作为整个表达式的结果
        current_block->instructions.push_back({ Instruction::ASSIGN, res, m_result_op });
        current_block->instructions.push_back({ Instruction::JUMP, {}, end_label });

        // 短路发生时，直接设置结果
        add_block(short_circuit_block);
        int short_val = (node->op == BinOp::LAnd) ? 0 : 1;
        current_block->instructions.push_back({ Instruction::ASSIGN, res, {Operand::CONST, "", 0, short_val} });

        add_block(end_block);
        m_result_op = res;
        return;
    }

    // --- 处理普通二元运算 ---
    node->lhs->accept(this);
    Operand lhs_op = m_result_op;

    node->rhs->accept(this);
    Operand rhs_op = m_result_op;

    m_result_op = new_temp();
    current_block->instructions.push_back({ map_bin_op(node->op), m_result_op, lhs_op, rhs_op });
}

void IRGenerator::visit(UnaryExpr* node) {
    node->sub->accept(this);
    Operand sub_op = m_result_op;

    m_result_op = new_temp();

    if (node->op == UnOp::Not) {
        current_block->instructions.push_back({ Instruction::NOT, m_result_op, sub_op });
    }
    else if (node->op == UnOp::Neg) {
        Operand zero;
        zero.kind = Operand::CONST;
        zero.value = 0;
        current_block->instructions.push_back({ Instruction::SUB, m_result_op, zero, sub_op });
    }
    else { // UnOp::Pos
        // 正号是无操作，直接传递子表达式的结果
        m_result_op = sub_op;
    }
}

void IRGenerator::visit(CallExpr* node) {
    // 1. 为所有参数生成操作数
    std::vector<Operand> args_ops;
    for (auto* arg_expr : node->args) {
        arg_expr->accept(this);
        args_ops.push_back(m_result_op);
    }

    // 2. 按照 C 语言的调用约定 (cdecl)，参数从右到左入栈。
    //    因此我们反向遍历参数，为每个参数生成一条 PARAM 指令。
    std::reverse(args_ops.begin(), args_ops.end());
    for (const auto& arg_op : args_ops) {
        current_block->instructions.push_back({ Instruction::PARAM, {}, arg_op });
    }

    // 3. 生成 CALL 指令
    Operand callee_op;
    callee_op.kind = Operand::VAR; // 函数名
    callee_op.name = node->callee;

    // 函数的返回值存入一个新的临时变量
    Operand result_op = new_temp();

    Instruction call_instr;
    call_instr.opcode = Instruction::CALL;
    call_instr.result = result_op;
    call_instr.arg1 = callee_op;
    // 将参数数量作为 arg2 传递，方便后续代码生成器知道要从栈上清理多少空间
    call_instr.arg2.kind = Operand::CONST;
    call_instr.arg2.value = node->args.size();

    current_block->instructions.push_back(call_instr);

    // 4. 将函数调用的返回值作为当前表达式的结果
    m_result_op = result_op;
}

