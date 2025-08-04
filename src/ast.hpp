// ast.hpp
#pragma once
#include <string>
#include <vector>
#include <memory>

struct Program;
class Visitor; // 前向声明
extern Program* g_root;
enum class TypeKind { TY_INT, TY_VOID };

struct Node { virtual ~Node() = default; 
	virtual void accept(Visitor* v) = 0; //accept,用于检查语义
};

// --- 表达式 ---
struct Expr : Node { TypeKind type_val = TypeKind::TY_INT;};
struct IntLiteral : Expr { int value; explicit IntLiteral(int v) : value(v) {} 
	void accept(Visitor* v) override;
};
struct VarExpr : Expr { std::string name; explicit VarExpr(std::string n) : name(std::move(n)) {} 
	void accept(Visitor* v) override;
};

enum class BinOp { Add, Sub, Mul, Div, Mod, Lt, Gt, Le, Ge, Eq, Neq, LAnd, LOr };
struct BinaryExpr : Expr { BinOp op; Expr* lhs; Expr* rhs; BinaryExpr(BinOp o, Expr* l, Expr* r) : op(o), lhs(l), rhs(r) {} 
	void accept(Visitor* v) override;
};

enum class UnOp { Pos, Neg, Not };
struct UnaryExpr : Expr { UnOp op; Expr* sub; explicit UnaryExpr(UnOp o, Expr* s) : op(o), sub(s) {} 
	void accept(Visitor* v) override;
};

struct CallExpr : Expr { std::string callee; std::vector<Expr*> args;
	void accept(Visitor* v) override;
};

// --- 语句 ---
struct Stmt : Node {
};
struct ExprStmt : Stmt { Expr* e; explicit ExprStmt(Expr* x) : e(x) {} 
	void accept(Visitor* v) override;
};
struct AssignStmt : Stmt { std::string name; Expr* rhs; AssignStmt(std::string n, Expr* r) : name(std::move(n)), rhs(r) {} 
	void accept(Visitor* v) override;
};
struct DeclStmt : Stmt { std::string name; Expr* init; DeclStmt(std::string n, Expr* i) : name(std::move(n)), init(i) {} 
	void accept(Visitor* v) override;
};
struct ReturnStmt : Stmt { Expr* e; explicit ReturnStmt(Expr* x) : e(x) {} 
	void accept(Visitor* v) override;
};
struct BreakStmt : Stmt {
	void accept(Visitor* v) override;
};
struct ContinueStmt : Stmt {
	void accept(Visitor* v) override;
};
struct Block : Stmt { std::vector<Stmt*> stmts; 
	void accept(Visitor* v) override;
};
struct IfStmt : Stmt {
	Expr* cond; Stmt* thenS; Stmt* elseS;
	IfStmt(Expr* c, Stmt* t, Stmt* e = nullptr) : cond(c), thenS(t), elseS(e) {}
	void accept(Visitor* v) override;
};

struct WhileStmt : Stmt {
	Expr* cond; Stmt* body;
	WhileStmt(Expr* c, Stmt* b) : cond(c), body(b) {}
	void accept(Visitor* v) override;
};


// --- 函数/程序 ---
struct Param : Node { TypeKind type_val; std::string name; 
	void accept(Visitor* v) override;
};
struct FuncDef : Node {
	TypeKind ret; std::string name; std::vector<Param*> params; Block* body;
	void accept(Visitor* v) override;
};
struct Program : Node { std::vector<FuncDef*> funcs; 
	void accept(Visitor* v) override;
};

// （可选）一个简单的“对象池”
extern std::vector<std::unique_ptr<Node>> g_arena;
template<class T, class...Args>
T* arena_make(Args&&...args) {
	auto p = std::make_unique<T>(std::forward<Args>(args)...);
	T* raw = p.get(); g_arena.emplace_back(std::move(p)); return raw;
}
class Visitor {
public:
	virtual ~Visitor() = default;
	// 为你的每一个 AST 节点类添加一个 visit 方法
	// 程序与函数
	virtual void visit(Program* node) = 0;
	virtual void visit(FuncDef* node) = 0;
	virtual void visit(Param* node) = 0;

	// 语句
	virtual void visit(Block* node) = 0;
	virtual void visit(ExprStmt* node) = 0;
	virtual void visit(AssignStmt* node) = 0;
	virtual void visit(DeclStmt* node) = 0;
	virtual void visit(ReturnStmt* node) = 0;
	virtual void visit(BreakStmt* node) = 0;
	virtual void visit(ContinueStmt* node) = 0;
	virtual void visit(IfStmt* node) = 0;
	virtual void visit(WhileStmt* node) = 0;

	// 表达式
	virtual void visit(IntLiteral* node) = 0;
	virtual void visit(VarExpr* node) = 0;
	virtual void visit(BinaryExpr* node) = 0;
	virtual void visit(UnaryExpr* node) = 0;
	virtual void visit(CallExpr* node) = 0;
};