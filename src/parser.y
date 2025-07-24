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
  ��һ�� %% ֮ǰ��������������ʡ�ԣ���
  �ڶ��� %% ֮ǰ�ǹ�������
  ���һ�� %% ֮���� C/C++ �ӳ����� 
%%*/

/* ��������һ����򵥵� start symbol */
%%
program:
     /* �ճ��� */
   | program statement
   ;

/* һ����򵥵� statement������������չ */
statement:
     |Block
     |SEMI
     |expression SEMI
     |IDENTIFIER ASSIGN expression SEMI
     |INT IDENTIFIER ASSIGN expression SEMI
     |IF LPAREN expression RPAREN statement  %prec LOWER_THAN_ELSE
     |IF LPAREN expression RPAREN statement ELSE statement
   ;
   
/* �򵥱��ʽʾ�� */
expression:
     NUMBER           { /* $1 ������ yylval */ }
   | expression PLUS expression
   | expression MINUS expression
   | LPAREN expression RPAREN
   ;
%%

/* ����ѡ���ṩһ����� main ������������ */
/*
int main() {
  return yyparse();
}
*/
