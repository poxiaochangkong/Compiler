%{ 
  // C/C++ ͷ�ļ���ȫ������
  #include <cstdio>
  // yylex �� lexer.l ������
  int yylex(void);        /* �������ɣ���� extern "C" */
  extern FILE* yyin;

  void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
  }
%}

/* token ������Ҫ�� lexer.l �� return �� TOKEN һһ��Ӧ */
/* ���� yylval ���Դ��������ַ���ָ�� */
%union {
  int    intval;
  char * strval;
}

/* ���� Bison����ͬ�� token ��Ӧ union �е��ĸ���Ա */
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


/*%%
  ��һ�� %% ֮ǰ��������������ʡ�ԣ���
  �ڶ��� %% ֮ǰ�ǹ�������
  ���һ�� %% ֮���� C/C++ �ӳ����� 
%%*/

/* ��������һ����򵥵� start symbol */
%%
program:
      FuncDef_list
   ;

FuncDef_list:
      FuncDef
    | FuncDef_list FuncDef   
    ;

/* һ����򵥵� statement������չ */
statement:
       Block
     | SEMI
     | expression SEMI
     | IDENTIFIER ASSIGN expression SEMI
     | INT IDENTIFIER ASSIGN expression SEMI
     | IF LPAREN expression RPAREN statement  %prec LOWER_THAN_ELSE
     | IF LPAREN expression RPAREN statement ELSE statement
     | WHILE LPAREN expression RPAREN statement 
     | BREAK SEMI
     | CONTINUE SEMI
     | RETURN SEMI
     | RETURN expression SEMI
   ;
statement_list:
    /*��*/
    | statement_list statement
    ;

Block:
      LBRACE statement_list RBRACE
    ;
   

FuncDef:
      return_type IDENTIFIER LPAREN param_list_opt RPAREN Block
    ;

return_type:
      INT
    | VOID
    ;

param_list_opt:
      /*��*/
    | param_list
    ;

param_list:
      param_list COMMA param
    | param
    ;

param:
      INT IDENTIFIER
    ;
    
expression:
      LOrexpr
    ;

LOrexpr:
      LAndexpr
    | LOrexpr OR LAndexpr
    ;

LAndexpr:
      Relexpr
    | LAndexpr AND Relexpr
    ;

Relexpr:
      Addexpr
    | Relexpr LT Addexpr
    | Relexpr GT Addexpr
    | Relexpr LE Addexpr
    | Relexpr GE Addexpr
    | Relexpr EQ Addexpr
    | Relexpr NEQ Addexpr
    ;

Addexpr:
      Mulexpr
    | Addexpr PLUS Mulexpr
    | Addexpr MINUS Mulexpr
    ; 

Mulexpr:
      Unaryexpr
    | Mulexpr MULTIPLY Unaryexpr
    | Mulexpr DIVIDE Unaryexpr
    | Mulexpr PERCENT Unaryexpr
    ;

Unaryexpr:
      Primaryexpr
    | PLUS Unaryexpr
    | MINUS Unaryexpr
    | EXCLAPOINT Unaryexpr
    ;

Primaryexpr:
      IDENTIFIER
    | NUMBER
    | LPAREN expression RPAREN
    | IDENTIFIER LPAREN expr_list_opt RPAREN
    ;

expr_list_opt:
      /*��*/
    | expr_list
    ;

expr_list:
      expr_list COMMA expression
    | expression
    ;
%%

/* ����ѡ���ṩһ����� main ������������ */
/*
int main() {
  return yyparse();
}
*/
