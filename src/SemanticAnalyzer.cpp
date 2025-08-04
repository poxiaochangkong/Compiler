// SemanticAnalyzer.cpp

#include "SemanticAnalyzer.hpp"

// --- ���ű���������ʵ�� ---

// �������
void SemanticAnalyzer::analyze(Program* root) {
    if (root) {
        root->accept(this);
    }
}

// ������������
void SemanticAnalyzer::enter_scope() {
    scopes.push_back({});
}

// �˳�������
void SemanticAnalyzer::exit_scope() {
    scopes.pop_back();
}

// �ڵ�ǰ�����������·���
bool SemanticAnalyzer::declare(const std::string& name, SymbolInfo info) {
    if (scopes.back().count(name)) {
        std::cerr << "Semantic Error: Redefinition of '" << name << "' in the same scope." << std::endl;
        return false;
    }
    scopes.back()[name] = info;
    return true;
}

// ����������ҷ���
SymbolInfo* SemanticAnalyzer::lookup(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto& scope = *it;
        auto found = scope.find(name);
        if (found != scope.end()) {
            return &found->second;
        }
    }
    return nullptr;
}


// --- Visitor ����������ʵ�� ---

// �����뺯��
void SemanticAnalyzer::visit(Program* node) {
    enter_scope(); // ����ȫ��������
    for (auto* func : node->funcs) {
        declare(func->name, { func->ret });
    }
    for (auto* func : node->funcs) {
        func->accept(this);
    }
    exit_scope(); // �˳�ȫ��������
}

void SemanticAnalyzer::visit(FuncDef* node) {
    current_function = node;
    enter_scope();
    for (auto* param : node->params) {
        param->accept(this);
    }
    node->body->accept(this);
    exit_scope();
    current_function = nullptr;
}

void SemanticAnalyzer::visit(Param* node) {
    // �����������߼����� FuncDef �д�������ȷ��������ȷ����
    // ���δ������֧�ָ����ӵ����ͣ������ڴ˴���
    declare(node->name, { node->type_val });
}

// ���
void SemanticAnalyzer::visit(Block* node) {
    enter_scope();
    for (auto* stmt : node->stmts) {
        if (stmt) stmt->accept(this);
    }
    exit_scope();
}

void SemanticAnalyzer::visit(ExprStmt* node) {
    if (node->e) {
        node->e->accept(this);
    }
}

void SemanticAnalyzer::visit(AssignStmt* node) {
    // ����������
    SymbolInfo* info = lookup(node->name);
    if (!info) {
        std::cerr << "Semantic Error: Use of undeclared identifier '" << node->name << "' in assignment." << std::endl;
        return;
    }

    // �����Ҳ���ʽ
    node->rhs->accept(this);

    // ���ͼ�� (�������������б����ͱ��ʽ����int)
    if (info->type != node->rhs->type_val) {
        std::cerr << "Semantic Error: Type mismatch in assignment to '" << node->name << "'." << std::endl;
    }
}

void SemanticAnalyzer::visit(DeclStmt* node) {
    if (node->init) {
        node->init->accept(this);
        if (node->init->type_val != TypeKind::TY_INT) {
            std::cerr << "Semantic Error: Initializer for variable '" << node->name << "' is not an integer." << std::endl;
        }
    }
    if (!declare(node->name, { TypeKind::TY_INT })) {
        // Redefinition error is already printed by declare()
    }
}

void SemanticAnalyzer::visit(ReturnStmt* node) {
    if (node->e) { // return <expr>;
        if (!current_function || current_function->ret == TypeKind::TY_VOID) {
            std::cerr << "Semantic Error: Returning a value from a void function." << std::endl;
        }
        node->e->accept(this);
        if (current_function && node->e->type_val != current_function->ret) {
            std::cerr << "Semantic Error: Return type mismatch in function." << std::endl;
        }
    }
    else { // return;
        if (current_function && current_function->ret != TypeKind::TY_VOID) {
            std::cerr << "Semantic Error: Non-void function must return a value." << std::endl;
        }
    }
}

void SemanticAnalyzer::visit(BreakStmt* node) {
    if (loop_depth <= 0) {
        std::cerr << "Semantic Error: 'break' statement not in a loop." << std::endl;
    }
}

void SemanticAnalyzer::visit(ContinueStmt* node) {
    if (loop_depth <= 0) {
        std::cerr << "Semantic Error: 'continue' statement not in a loop." << std::endl;
    }
}

void SemanticAnalyzer::visit(IfStmt* node) {
    node->cond->accept(this);
    // �����ڴ˼���������ͣ����� if (node->cond->type_val != TypeKind::TY_INT) { ... }

    node->thenS->accept(this);
    if (node->elseS) {
        node->elseS->accept(this);
    }
}

void SemanticAnalyzer::visit(WhileStmt* node) {
    node->cond->accept(this);

    loop_depth++;
    node->body->accept(this);
    loop_depth--;
}

// ���ʽ
void SemanticAnalyzer::visit(IntLiteral* node) {
    node->type_val = TypeKind::TY_INT;
}

void SemanticAnalyzer::visit(VarExpr* node) {
    SymbolInfo* info = lookup(node->name);
    if (!info) {
        std::cerr << "Semantic Error: Use of undeclared identifier '" << node->name << "'." << std::endl;
        node->type_val = TypeKind::TY_INT; // ����ʱ�ٶ�һ�������Լ���
    }
    else {
        node->type_val = info->type;
    }
}

void SemanticAnalyzer::visit(BinaryExpr* node) {
    node->lhs->accept(this);
    node->rhs->accept(this);
    if (node->lhs->type_val != TypeKind::TY_INT || node->rhs->type_val != TypeKind::TY_INT) {
        std::cerr << "Semantic Error: Operands of binary expression must be integers." << std::endl;
    }
    node->type_val = TypeKind::TY_INT;
}

void SemanticAnalyzer::visit(UnaryExpr* node) {
    node->sub->accept(this);
    if (node->sub->type_val != TypeKind::TY_INT) {
        std::cerr << "Semantic Error: Operand of unary expression must be an integer." << std::endl;
    }
    node->type_val = TypeKind::TY_INT;
}

void SemanticAnalyzer::visit(CallExpr* node) {
    SymbolInfo* info = lookup(node->callee);
    if (!info) {
        std::cerr << "Semantic Error: Call to undeclared function '" << node->callee << "'." << std::endl;
        node->type_val = TypeKind::TY_INT; // �ٶ�һ������
        return;
    }
    // (��ѡ) ����������������
    // ...

    for (auto* arg : node->args) {
        arg->accept(this);
    }
    node->type_val = info->type;
}