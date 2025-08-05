#include "IRGenerator.hpp"
#include <stdexcept> // ���� std::runtime_error
#include <string>    // ���� std::to_string

ModuleIR IRGenerator::generate(Program* root) {
    if (root) {
        root->accept(this);
    }
    return m_module;
}

// --- ��������ʵ�� ---

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
        // LAnd �� LOr ͨ����·��ֵ������ֱ��ӳ��
        throw std::runtime_error("Unhandled binary operator for direct mapping");
    }
}

// --- Visitor ����ʵ�� ---

void IRGenerator::visit(Program* node) {
    for (auto* func : node->funcs) {
        func->accept(this);
    }
}

void IRGenerator::visit(FuncDef* node) {
    m_module.functions.emplace_back();
    current_func = &m_module.functions.back();
    current_func->name = node->name;

    // ÿ����������һ����ڿ�
    BasicBlock* entry_block = create_block(".entry_");
    add_block(entry_block);

    // Ϊ�����������ɶ�Ӧ�� IR ����������Ȼ������׶β�ֱ������ָ�
    for (auto* param : node->params) {
        param->accept(this);
    }

    node->body->accept(this);

    // ȷ���� void ����������·�����з��أ�����һ���򻯵ļ�飩
    // һ���������ļ����Ҫ����������ͼ
    if (current_block && (current_block->instructions.empty() || current_block->instructions.back().opcode != Instruction::RET)) {
        if (node->ret == TypeKind::TY_VOID) {
            current_block->instructions.push_back({ Instruction::RET });
        }
    }
    current_func = nullptr;
}

void IRGenerator::visit(Param* node) {
    // �� IR ���ɽ׶Σ�������Ҫ���ں���ǩ����
    // ʵ�ʵĲ��������� visit(CallExpr*) �д���
    // �������ʲô��������
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
    // 1. �����Ҳ���ʽ��ֵ������� m_result_op ��
    node->rhs->accept(this);
    Operand src_op = m_result_op;

    // 2. ���������������Ĳ�����
    Operand dest_op;
    dest_op.kind = Operand::VAR;
    dest_op.name = node->name;

    // 3. ���ɸ�ֵָ��
    current_block->instructions.push_back({ Instruction::ASSIGN, dest_op, src_op });
}

void IRGenerator::visit(DeclStmt* node) {
    // ��������������г�ʼ�����͵�ͬ��һ�θ�ֵ
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

    // 1. �����������ʽ
    node->cond->accept(this);

    // 2. ����������תָ��
    Operand false_dest_label;
    false_dest_label.kind = Operand::LABEL;
    false_dest_label.name = else_block->label;
    current_block->instructions.push_back({ Instruction::JUMP_IF_ZERO, {}, m_result_op, false_dest_label });
    // �������Ϊ�棬��˳��ִ�е� then �飨��Ҫ����ת��
    Operand true_dest_label;
    true_dest_label.kind = Operand::LABEL;
    true_dest_label.name = then_block->label;
    current_block->instructions.push_back({ Instruction::JUMP, {}, true_dest_label });


    // 3. ���� Then ��
    add_block(then_block);
    node->thenS->accept(this);
    // Then ���������������ת�� Merge ��
    Operand merge_dest_label;
    merge_dest_label.kind = Operand::LABEL;
    merge_dest_label.name = merge_block->label;
    current_block->instructions.push_back({ Instruction::JUMP, {}, merge_dest_label });

    // 4. ���� Else �� (�������)
    if (node->elseS) {
        add_block(else_block);
        node->elseS->accept(this);
        // Else �������Ҳ��������ת�� Merge ��
        current_block->instructions.push_back({ Instruction::JUMP, {}, merge_dest_label });
    }

    // 5. ���������� Merge ��������
    add_block(merge_block);
}

void IRGenerator::visit(WhileStmt* node) {
    BasicBlock* cond_block = create_block(".while_cond_");
    BasicBlock* body_block = create_block(".while_body_");
    BasicBlock* end_block = create_block(".while_end_");

    // ���� break �� continue ����תĿ��
    Operand end_label_op;
    end_label_op.kind = Operand::LABEL;
    end_label_op.name = end_block->label;
    break_labels.push_back(end_label_op);

    Operand cond_label_op;
    cond_label_op.kind = Operand::LABEL;
    cond_label_op.name = cond_block->label;
    continue_labels.push_back(cond_label_op);

    // ��������ת�������жϿ�
    current_block->instructions.push_back({ Instruction::JUMP, {}, cond_label_op });

    // 1. ���������жϿ�
    add_block(cond_block);
    node->cond->accept(this);
    Operand body_label_op;
    body_label_op.kind = Operand::LABEL;
    body_label_op.name = body_block->label;
    current_block->instructions.push_back({ Instruction::JUMP_IF_NZERO, {}, m_result_op, body_label_op });
    // �������Ϊ�٣���ת��ѭ��������
    current_block->instructions.push_back({ Instruction::JUMP, {}, end_label_op });

    // 2. ����ѭ�����
    add_block(body_block);
    node->body->accept(this);
    // ѭ������������������������жϿ�
    current_block->instructions.push_back({ Instruction::JUMP, {}, cond_label_op });

    // 3. ����������ѭ��������������
    add_block(end_block);

    // �뿪ѭ����������Ӧ�ı�ǩ
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
    // --- �����·��ֵ ---
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

        // ���û�ж�·��������Ҳ���ʽ
        node->rhs->accept(this);
        // ���Ҳ���ʽ�Ľ����0���0����Ϊ�������ʽ�Ľ��
        current_block->instructions.push_back({ Instruction::ASSIGN, res, m_result_op });
        current_block->instructions.push_back({ Instruction::JUMP, {}, end_label });

        // ��·����ʱ��ֱ�����ý��
        add_block(short_circuit_block);
        int short_val = (node->op == BinOp::LAnd) ? 0 : 1;
        current_block->instructions.push_back({ Instruction::ASSIGN, res, {Operand::CONST, "", 0, short_val} });

        add_block(end_block);
        m_result_op = res;
        return;
    }

    // --- ������ͨ��Ԫ���� ---
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
        // �������޲�����ֱ�Ӵ����ӱ��ʽ�Ľ��
        m_result_op = sub_op;
    }
}

void IRGenerator::visit(CallExpr* node) {
    // 1. Ϊ���в������ɲ�����
    std::vector<Operand> args_ops;
    for (auto* arg_expr : node->args) {
        arg_expr->accept(this);
        args_ops.push_back(m_result_op);
    }

    // 2. ���� C ���Եĵ���Լ�� (cdecl)���������ҵ�����ջ��
    //    ������Ƿ������������Ϊÿ����������һ�� PARAM ָ�
    std::reverse(args_ops.begin(), args_ops.end());
    for (const auto& arg_op : args_ops) {
        current_block->instructions.push_back({ Instruction::PARAM, {}, arg_op });
    }

    // 3. ���� CALL ָ��
    Operand callee_op;
    callee_op.kind = Operand::VAR; // ������
    callee_op.name = node->callee;

    // �����ķ���ֵ����һ���µ���ʱ����
    Operand result_op = new_temp();

    Instruction call_instr;
    call_instr.opcode = Instruction::CALL;
    call_instr.result = result_op;
    call_instr.arg1 = callee_op;
    // ������������Ϊ arg2 ���ݣ������������������֪��Ҫ��ջ��������ٿռ�
    call_instr.arg2.kind = Operand::CONST;
    call_instr.arg2.value = node->args.size();

    current_block->instructions.push_back(call_instr);

    // 4. ���������õķ���ֵ��Ϊ��ǰ���ʽ�Ľ��
    m_result_op = result_op;
}

