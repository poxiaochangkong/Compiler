%{ 
  // C/C++ 头文件与全局声明
  #include <cstdio>
  #include "ast.hpp"
  // yylex 在 lexer.l 里生成
  int yylex(void);        /* 声明即可，别加 extern "C" */
  extern FILE* yyin;

  Program* g_root = nullptr;

  void yyerror(const char *s) {
    std::fprintf(stderr, "Parse error at line %d: %s\n", yylineno, s);
  }
%}

/* token 声明：要和 lexer.l 中 return 的 TOKEN 一一对应 */
/* 定义 yylval 可以存整数或字符串指针 */
%union {
   int                    intval;
  char*                  strval;

  TypeKind               ty;

  Expr*                  expr;
  Stmt*                  stmt;
  Block*                 block;
  FuncDef*               func;
  Param*                 param;
  Program*               program;

  std::vector<Stmt*>*        stmts;
  std::vector<Expr*>*        args;
  std::vector<Param*>*       params;
  std::vector<FuncDef*>*     funcs;
}

/* 告诉 Bison：不同的 token 对应 union 中的哪个成员 */
%token <intval>    NUMBER
%token <strval>    IDENTIFIER
%token              PLUS MINUS MULTIPLY DIVIDE PERCENT
%token              EXCLAPOINT EQ NEQ LE GE LT GT OR AND
%token              ASSIGN
%token              LPAREN RPAREN SEMI LBRACE RBRACE COMMA
%token              INT VOID
%token              IF ELSE WHILE BREAK CONTINUE
%token              RETURN
%token              ERROR

%nonassoc LOWER_THAN_ELSE   
%nonassoc ELSE              

/* -------------------- 非终结符类型 -------------------- */
%type <program> program
%type <funcs>   FuncDef_list
%type <func>    FuncDef

%type <stmt>    statement
%type <stmts>   statement_list
%type <block>   Block

%type <ty>      return_type
%type <params>  param_list param_list_opt
%type <param>   param

%type <expr>    expression LOrexpr LAndexpr Relexpr Addexpr Mulexpr Unaryexpr Primaryexpr
%type <args>    expr_list expr_list_opt

/* -------------------- 析构器（出错回溯时释放中间值） -------------------- */
%destructor { free($$); } <strval>
%destructor { delete $$; } <stmts> <args> <params> <funcs>
/*%%
  第一对 %% 之前是声明区（可以省略），
  第二对 %% 之前是规则区，
  最后一个 %% 之后是 C/C++ 子程序区 
%%*/

/* 规则区*/
%%
program:
      FuncDef_list
      {
      Program* p = arena_make<Program>();
      if ($1) { p->funcs.swap(*$1); delete $1; }
      $$ = p;
      g_root = p;
    }
   ;

FuncDef_list:
      FuncDef
      {
      $$ = new std::vector<FuncDef*>();
      $$->push_back($1);
    }
    | FuncDef_list FuncDef 
    {
      $$ = $1;//$$为最左的FuncDef规约得到的FuncDef_list
      $$->push_back($2);//压入
    }
    ;


statement:
       Block{ $$ = $1; }
     | SEMI { $$ = nullptr; }
     | expression SEMI { $$ = arena_make<ExprStmt>($1); }
     | IDENTIFIER ASSIGN expression SEMI{
      $$ = arena_make<AssignStmt>(std::string($1), $3);
      free($1);
    }
     | INT IDENTIFIER ASSIGN expression SEMI
     {
      /* 局部变量类型固定为 int */
      $$ = arena_make<DeclStmt>(std::string($2), $4); free($2);
    }
     | IF LPAREN expression RPAREN statement  %prec LOWER_THAN_ELSE
     {
      $$ = arena_make<IfStmt>($3, $5, nullptr);
    }
     | IF LPAREN expression RPAREN statement ELSE statement
     {
      $$ = arena_make<IfStmt>($3, $5, $7);
    }
     | WHILE LPAREN expression RPAREN statement 
     {
      $$ = arena_make<WhileStmt>($3, $5);
    }
     | BREAK SEMI
      { $$ = arena_make<BreakStmt>(); }
     | CONTINUE SEMI { $$ = arena_make<ContinueStmt>(); }
     | RETURN SEMI { $$ = arena_make<ReturnStmt>(nullptr); }
     | RETURN expression SEMI  { $$ = arena_make<ReturnStmt>($2); }
   ;
statement_list:
    /*ε*/{ $$ = new std::vector<Stmt*>(); }
    | statement_list statement{ $$ = $1; if ($2) $$->push_back($2); }
    ;

Block:
      LBRACE statement_list RBRACE
      {
      Block* b = arena_make<Block>();
      if ($2) {
        for (auto* s : *$2) if (s) b->stmts.push_back(s);
        delete $2;
      }
      $$ = b;
    }
    ;
   

FuncDef:
      return_type IDENTIFIER LPAREN param_list_opt RPAREN Block
      {
      FuncDef* f = arena_make<FuncDef>();
      f->ret = $1;
      f->name = std::string($2); free($2);
      if ($4) { f->params.swap(*$4); delete $4; }
      f->body = $6;
      $$ = f;
    }
    ;

return_type:
      INT{ $$ = TypeKind::TY_INT; }
    | VOID{ $$ = TypeKind::TY_VOID; }
    ;

param_list_opt:
      /*ε*/{ $$ = new std::vector<Param*>(); }
    | param_list{ $$ = $1; }
    ;

param_list:
      param_list COMMA param
      {
      $$ = $1;
      $$->push_back($3);
    }
    | param
    {
      $$ = new std::vector<Param*>();
      $$->push_back($1);
    }
    ;

param:
      INT IDENTIFIER
      {
      Param* p = arena_make<Param>();
      p->ty = TypeKind::TY_INT;
      p->name = std::string($2); free($2);
      $$ = p;
    }
    ;
    
expression:
      LOrexpr{ $$ = $1; }
    ;

LOrexpr:
      LAndexpr { $$ = $1; }
    | LOrexpr OR LAndexpr{ $$ = arena_make<BinaryExpr>(BinOp::LOr,  $1, $3); }
    ;

LAndexpr:
      Relexpr{ $$ = $1; }
    | LAndexpr AND Relexpr{ $$ = arena_make<BinaryExpr>(BinOp::LAnd, $1, $3); }
    ;

Relexpr:
      Addexpr{ $$ = $1; }   
    | Relexpr LT Addexpr { $$ = arena_make<BinaryExpr>(BinOp::Lt,  $1, $3); }
    | Relexpr GT Addexpr { $$ = arena_make<BinaryExpr>(BinOp::Gt,  $1, $3); }
    | Relexpr LE Addexpr { $$ = arena_make<BinaryExpr>(BinOp::Le,  $1, $3); }
    | Relexpr GE Addexpr { $$ = arena_make<BinaryExpr>(BinOp::Ge,  $1, $3); }
    | Relexpr EQ Addexpr { $$ = arena_make<BinaryExpr>(BinOp::Eq,  $1, $3); }
    | Relexpr NEQ Addexpr { $$ = arena_make<BinaryExpr>(BinOp::Neq, $1, $3); }
    ;

Addexpr:
      Mulexpr{ $$ = $1; }
    | Addexpr PLUS Mulexpr{ $$ = arena_make<BinaryExpr>(BinOp::Add, $1, $3); }
    | Addexpr MINUS Mulexpr{ $$ = arena_make<BinaryExpr>(BinOp::Sub, $1, $3); }
    ; 

Mulexpr:
      Unaryexpr{ $$ = $1; }
    | Mulexpr MULTIPLY Unaryexpr { $$ = arena_make<BinaryExpr>(BinOp::Mul, $1, $3); }
    | Mulexpr DIVIDE Unaryexpr { $$ = arena_make<BinaryExpr>(BinOp::Div, $1, $3); }
    | Mulexpr PERCENT Unaryexpr { $$ = arena_make<BinaryExpr>(BinOp::Mod, $1, $3); }
    ;

Unaryexpr:
      Primaryexpr { $$ = $1; }
    | PLUS Unaryexpr { $$ = arena_make<UnaryExpr>(UnOp::Pos, $2); }
    | MINUS Unaryexpr { $$ = arena_make<UnaryExpr>(UnOp::Neg, $2); }
    | EXCLAPOINT Unaryexpr { $$ = arena_make<UnaryExpr>(UnOp::Not, $2); }
    ;

Primaryexpr:
      IDENTIFIER
      {
      $$ = arena_make<VarExpr>(std::string($1)); free($1);
    }

    | NUMBER
    {
      $$ = arena_make<IntLiteral>($1);
    }
    | LPAREN expression RPAREN
    {
      $$ = $2;
    }
    | IDENTIFIER LPAREN expr_list_opt RPAREN
    {
      CallExpr* c = arena_make<CallExpr>();
      c->callee = std::string($1); free($1);
      if ($3) { c->args.swap(*$3); delete $3; }
      $$ = c;
    }
    ;

expr_list_opt:
      /*ε*/{ $$ = new std::vector<Expr*>(); }
    | expr_list{ $$ = $1; }
    ;

expr_list:
      expr_list COMMA expression
      {
      $$ = $1;
      $$->push_back($3);
    }
    | expression
    {
      $$ = new std::vector<Expr*>();
      $$->push_back($1);
    }
    ;
%%

/* （可选）提供一个入口 main 给单独测试用 */
/* 
int main() {
  return yyparse();
}
*/
