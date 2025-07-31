// ast.hpp
#pragma once
#include <string>
#include <vector>
#include <memory>

struct Program;
extern Program* g_root;
enum class TypeKind { TY_INT, TY_VOID };

struct Node { virtual ~Node() = default; };

// --- 表达式 ---
struct Expr : Node { TypeKind type_val = TypeKind::TY_INT; };
struct IntLiteral : Expr { int value; explicit IntLiteral(int v) : value(v) {} };
struct VarExpr : Expr { std::string name; explicit VarExpr(std::string n) : name(std::move(n)) {} };

enum class BinOp { Add, Sub, Mul, Div, Mod, Lt, Gt, Le, Ge, Eq, Neq, LAnd, LOr };
struct BinaryExpr : Expr { BinOp op; Expr* lhs; Expr* rhs; BinaryExpr(BinOp o, Expr* l, Expr* r) : op(o), lhs(l), rhs(r) {} };

enum class UnOp { Pos, Neg, Not };
struct UnaryExpr : Expr { UnOp op; Expr* sub; explicit UnaryExpr(UnOp o, Expr* s) : op(o), sub(s) {} };

struct CallExpr : Expr { std::string callee; std::vector<Expr*> args; };

// --- 语句 ---
struct Stmt : Node {};
struct ExprStmt : Stmt { Expr* e; explicit ExprStmt(Expr* x) : e(x) {} };
struct AssignStmt : Stmt { std::string name; Expr* rhs; AssignStmt(std::string n, Expr* r) : name(std::move(n)), rhs(r) {} };
struct DeclStmt : Stmt { std::string name; Expr* init; DeclStmt(std::string n, Expr* i) : name(std::move(n)), init(i) {} };
struct ReturnStmt : Stmt { Expr* e; explicit ReturnStmt(Expr* x) : e(x) {} };
struct BreakStmt : Stmt {};
struct ContinueStmt : Stmt {};
struct Block : Stmt { std::vector<Stmt*> stmts; };
struct IfStmt : Stmt {
	Expr* cond; Stmt* thenS; Stmt* elseS;
	IfStmt(Expr* c, Stmt* t, Stmt* e = nullptr) : cond(c), thenS(t), elseS(e) {}
};

struct WhileStmt : Stmt {
	Expr* cond; Stmt* body;
	WhileStmt(Expr* c, Stmt* b) : cond(c), body(b) {}
};


// --- 函数/程序 ---
struct Param : Node { TypeKind type_val; std::string name; };
struct FuncDef : Node {
	TypeKind ret; std::string name; std::vector<Param*> params; Block* body;
};
struct Program : Node { std::vector<FuncDef*> funcs; };

// （可选）一个简单的“对象池”
extern std::vector<std::unique_ptr<Node>> g_arena;
template<class T, class...Args>
T* arena_make(Args&&...args) {
	auto p = std::make_unique<T>(std::forward<Args>(args)...);
	T* raw = p.get(); g_arena.emplace_back(std::move(p)); return raw;
}
