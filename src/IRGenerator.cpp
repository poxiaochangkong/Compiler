#include "IRGenerator.hpp"
#include <stdexcept> // 用于 std::runtime_error
#include <string>    // 用于 std::to_string
#include <algorithm>

ModuleIR IRGenerator::generate(Program* root) {
    if (root) {
        root->accept(this);
    }
    return m_module;
}

void IRGenerator::enter_scope() {
    m_scopes.push_back({});
}

void IRGenerator::exit_scope() {
    if (!m_scopes.empty()) {
        m_scopes.pop_back();
    }
}

// 为变量声明创建唯一的 IR 操作数
Operand IRGenerator::declare_variable(const std::string& name) {
    if (m_scopes.empty()) {
        // 这是一个防御性检查，正常情况下函数定义会创建第一个作用域
        enter_scope();
    }

    Operand op;
    op.kind = Operand::VAR;
    // 通过计数器为变量名添加后缀，使其唯一，例如 "x" -> "x_0"
    op.name = name + "_" + std::to_string(m_var_counter++);

    // 在当前作用域的 map 中，将原始名称映射到这个唯一的 Operand
    m_scopes.back()[name] = op;

    return op;
}

// 查找变量对应的唯一 IR 操作数
Operand IRGenerator::lookup_variable(const std::string& name) {
    // 从内层作用域向外层查找
    for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
        const auto& scope = *it;
        if (scope.count(name)) {
            return scope.at(name);
        }
    }
    // 如果代码通过了语义分析，这里本不应该执行到
    throw std::runtime_error("IRGen Error: Undeclared variable '" + name + "'.");
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

    // 为每个新函数重置作用域和变量计数器
    m_scopes.clear();
    m_var_counter = 0;
    enter_scope(); // 创建函数顶级作用域

    // 每个函数都有一个入口块
    BasicBlock* entry_block = create_block(".entry_");
    add_block(entry_block);

    for (auto* param_ast : node->params) {
        Operand unique_param_op = declare_variable(param_ast->name);
        current_func->params.push_back({ unique_param_op.name, });
    }

    node->body->accept(this);

    // 确保非 void 函数的所有路径都有返回
    if (current_block && (current_block->instructions.empty() || current_block->instructions.back().opcode != Instruction::RET)) {
        if (node->ret == TypeKind::TY_VOID) {
            current_block->instructions.push_back({ Instruction::RET });
        }
    }
    exit_scope(); // 退出函数作用域
    current_func = nullptr;
}

void IRGenerator::visit(Param* node) {
    // This logic is now handled in visit(FuncDef*)
}

void IRGenerator::visit(Block* node) {
    enter_scope();
    for (auto* stmt : node->stmts) {
        // 如果 current_block 为 nullptr，说明我们处于死代码区域，可以停止生成
        if (stmt && current_block) stmt->accept(this);
    }
    exit_scope();
}

void IRGenerator::visit(ExprStmt* node) {
    if (node->e) {
        node->e->accept(this);
    }
}

void IRGenerator::visit(AssignStmt* node) {
    node->rhs->accept(this);
    Operand rhs = m_result_op;
    Operand lhs = lookup_variable(node->name);
    current_block->instructions.push_back({ Instruction::ASSIGN, lhs, rhs });
}

void IRGenerator::visit(DeclStmt* node) {
    node->init->accept(this);
    Operand rhs = m_result_op;
    Operand lhs = declare_variable(node->name);
    current_block->instructions.push_back({ Instruction::ASSIGN, lhs, rhs });
}

// =================================================================
// vvvvvvvvvvvvvvvv         修复开始         vvvvvvvvvvvvvvvvvv
// =================================================================

void IRGenerator::visit(ReturnStmt* node) {
    Instruction instr;
    instr.opcode = Instruction::RET;
    if (node->e) {
        node->e->accept(this);
        instr.arg1 = m_result_op;
    }
    current_block->instructions.push_back(instr);

    // 【修复】生成终结者指令后，切换到一个新的、不可达的块
    if (current_func) {
        BasicBlock* unreachable_block = create_block(".unreachable_");
        add_block(unreachable_block);
    }
}

void IRGenerator::visit(BreakStmt* node) {
    if (!break_labels.empty()) {
        current_block->instructions.push_back({ Instruction::JUMP, {}, break_labels.back() });

        // 【修复】生成终结者指令后，切换到一个新的、不可达的块
        if (current_func) {
            BasicBlock* unreachable_block = create_block(".unreachable_");
            add_block(unreachable_block);
        }
    }
}

void IRGenerator::visit(ContinueStmt* node) {
    if (!continue_labels.empty()) {
        current_block->instructions.push_back({ Instruction::JUMP, {}, continue_labels.back() });

        // 【修复】生成终结者指令后，切换到一个新的、不可达的块
        if (current_func) {
            BasicBlock* unreachable_block = create_block(".unreachable_");
            add_block(unreachable_block);
        }
    }
}

// =================================================================
// ^^^^^^^^^^^^^^^^          修复结束          ^^^^^^^^^^^^^^^^^^
// =================================================================


void IRGenerator::visit(IfStmt* node) {
    BasicBlock* then_block = create_block();
    BasicBlock* merge_block = create_block();
    BasicBlock* else_block = node->elseS ? create_block() : nullptr;

    Operand false_dest_label;
    false_dest_label.kind = Operand::LABEL;
    false_dest_label.name = else_block ? else_block->label : merge_block->label;

    node->cond->accept(this);

    current_block->instructions.push_back({ Instruction::JUMP_IF_ZERO, {}, m_result_op, false_dest_label });

    add_block(then_block);
    node->thenS->accept(this);

    // 只有当当前块没有被终结时，才添加跳向 merge_block 的 JUMP
    if (current_block && (current_block->instructions.empty() ||
        (current_block->instructions.back().opcode != Instruction::RET &&
            current_block->instructions.back().opcode != Instruction::JUMP))) {
        current_block->instructions.push_back({ Instruction::JUMP, {}, {Operand::LABEL, merge_block->label} });
    }

    if (else_block) {
        add_block(else_block);
        node->elseS->accept(this);
        // 同样，检查 else 块是否已经被终结
        if (current_block && (current_block->instructions.empty() ||
            (current_block->instructions.back().opcode != Instruction::RET &&
                current_block->instructions.back().opcode != Instruction::JUMP))) {
            current_block->instructions.push_back({ Instruction::JUMP, {}, {Operand::LABEL, merge_block->label} });
        }
    }

    add_block(merge_block);
}

void IRGenerator::visit(WhileStmt* node) {
    BasicBlock* cond_block = create_block(".while_cond_");
    BasicBlock* body_block = create_block(".while_body_");
    BasicBlock* end_block = create_block(".while_end_");

    Operand end_label_op;
    end_label_op.kind = Operand::LABEL;
    end_label_op.name = end_block->label;
    break_labels.push_back(end_label_op);

    Operand cond_label_op;
    cond_label_op.kind = Operand::LABEL;
    cond_label_op.name = cond_block->label;
    continue_labels.push_back(cond_label_op);

    current_block->instructions.push_back({ Instruction::JUMP, {}, cond_label_op });

    add_block(cond_block);
    node->cond->accept(this);
    Operand body_label_op;
    body_label_op.kind = Operand::LABEL;
    body_label_op.name = body_block->label;
    // JUMP_IF_NZERO (如果非0则跳转) -> body
    current_block->instructions.push_back({ Instruction::JUMP_IF_NZERO, {}, m_result_op, body_label_op });
    // 如果为0，则自然掉落到下一条指令，即跳转到 end
    current_block->instructions.push_back({ Instruction::JUMP, {}, end_label_op });

    add_block(body_block);
    node->body->accept(this);
    // 只有当循环体块没有被终结时，才跳回条件判断
    if (current_block && (current_block->instructions.empty() ||
        (current_block->instructions.back().opcode != Instruction::RET &&
            current_block->instructions.back().opcode != Instruction::JUMP))) {
        current_block->instructions.push_back({ Instruction::JUMP, {}, cond_label_op });
    }

    add_block(end_block);

    break_labels.pop_back();
    continue_labels.pop_back();
}

void IRGenerator::visit(IntLiteral* node) {
    m_result_op.kind = Operand::CONST;
    m_result_op.value = node->value;
}

void IRGenerator::visit(VarExpr* node) {
    m_result_op = lookup_variable(node->name);
}

void IRGenerator::visit(BinaryExpr* node) {
    if (node->op == BinOp::LAnd || node->op == BinOp::LOr) {
        Operand res = new_temp();
        BasicBlock* rhs_block = create_block();
        BasicBlock* set_true_block = create_block();
        BasicBlock* set_false_block = create_block();
        BasicBlock* end_block = create_block();

        Operand rhs_label; rhs_label.kind = Operand::LABEL; rhs_label.name = rhs_block->label;
        Operand set_true_label; set_true_label.kind = Operand::LABEL; set_true_label.name = set_true_block->label;
        Operand set_false_label; set_false_label.kind = Operand::LABEL; set_false_label.name = set_false_block->label;
        Operand end_label; end_label.kind = Operand::LABEL; end_label.name = end_block->label;

        node->lhs->accept(this);
        Operand lhs_val = m_result_op;

        if (node->op == BinOp::LAnd) {
            current_block->instructions.push_back({ Instruction::JUMP_IF_ZERO, {}, lhs_val, set_false_label });
        }
        else { // BinOp::LOr
            current_block->instructions.push_back({ Instruction::JUMP_IF_NZERO, {}, lhs_val, set_true_label });
        }
        add_block(rhs_block);

        node->rhs->accept(this);
        Operand rhs_val = m_result_op;
        current_block->instructions.push_back({ Instruction::JUMP_IF_ZERO, {}, rhs_val, set_false_label });
        current_block->instructions.push_back({ Instruction::JUMP, {}, set_true_label });

        add_block(set_true_block);
        current_block->instructions.push_back({ Instruction::ASSIGN, res, {Operand::CONST, "", 0, 1} });
        current_block->instructions.push_back({ Instruction::JUMP, {}, end_label });

        add_block(set_false_block);
        current_block->instructions.push_back({ Instruction::ASSIGN, res, {Operand::CONST, "", 0, 0} });
        current_block->instructions.push_back({ Instruction::JUMP, {}, end_label });

        add_block(end_block);
        m_result_op = res;
        return;
    }

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
        m_result_op = sub_op;
    }
}

void IRGenerator::visit(CallExpr* node) {
    std::vector<Operand> args_ops;
    for (auto* arg_expr : node->args) {
        arg_expr->accept(this);
        args_ops.push_back(m_result_op);
    }

    for (const auto& arg_op : args_ops) {
        current_block->instructions.push_back({ Instruction::PARAM, {}, arg_op });
    }

    Operand callee_op;
    callee_op.kind = Operand::VAR;
    callee_op.name = node->callee;

    Operand result_op = new_temp();

    Instruction call_instr;
    call_instr.opcode = Instruction::CALL;
    call_instr.result = result_op;
    call_instr.arg1 = callee_op;
    call_instr.arg2.kind = Operand::CONST;
    call_instr.arg2.value = node->args.size();

    current_block->instructions.push_back(call_instr);

    m_result_op = result_op;
}