%{ 
  // C/C++ 头文件与全局声明
  #include <cstdio>
  // yylex 在 lexer.l 里生成
  int yylex(void);        /* 声明即可，别加 extern "C" */
  extern FILE* yyin;

  void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
  }
%}

/* token 声明：要和 lexer.l 中 return 的 TOKEN 一一对应 */
/* 定义 yylval 可以存整数或字符串指针 */
%union {
  int    intval;
  char * strval;
}

/* 告诉 Bison：不同的 token 对应 union 中的哪个成员 */
%token <intval>    NUMBER
%token <strval>    IDENTIFIER
%token              PLUS MINUS MULTIPLY DIVIDE 
%token              EXCLAPOINT EQ LE GE LT GT OR AND
%token              ASSIGN
%token              LPAREN RPAREN SEMI LBRACE RBRACE COMMA
%token              INT VOID
%token              IF ELSE WHILE BREAK CONTINUE
%token              RETURN

%nonassoc LOWER_THAN_ELSE   
%nonassoc ELSE              


/*%%
  第一对 %% 之前是声明区（可以省略），
  第二对 %% 之前是规则区，
  最后一个 %% 之后是 C/C++ 子程序区 
%%*/

/* 规则区：一个最简单的 start symbol */
%%
program:
     /* 空程序 */
   | program statement
   ;

/* 一个最简单的 statement，后面你再扩展 */
statement:
     |Block
     |SEMI
     |expression SEMI
     |IDENTIFIER ASSIGN expression SEMI
     |INT IDENTIFIER ASSIGN expression SEMI
     |IF LPAREN expression RPAREN statement  %prec LOWER_THAN_ELSE
     |IF LPAREN expression RPAREN statement ELSE statement
   ;
   
/* 简单表达式示例 */
expression:
     NUMBER           { /* $1 存在于 yylval */ }
   | expression PLUS expression
   | expression MINUS expression
   | LPAREN expression RPAREN
   ;
%%

/* （可选）提供一个入口 main 给单独测试用 */
/*
int main() {
  return yyparse();
}
*/
