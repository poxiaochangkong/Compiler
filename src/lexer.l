%{ 
  /* —— Definitions 区 —— */

  /* 在 Windows 下阻止包含 <unistd.h>，并让 isatty 可用 */
  #define YY_NO_UNISTD_H
  #ifdef _MSC_VER
    #include <io.h>
    #define isatty _isatty
    #define fileno _fileno
    #define strdup _strdup
  #endif

  /* 引入 Bison 生成的头，内含 union YYSTYPE 和 extern YYSTYPE yylval */
  #include "parser.tab.h"
  #define YY_DECL int yylex(void)
  #include <cstdlib>   /* std::atoi */
  #include <cstdio>    /* fopen, perror, fprintf */
  #include <cstring>   /* strdup */
  /* Flex 自动声明：yyin 指向当前输入流，yylineno 记录行号 */
  
%}

/* 禁用默认 yywrap()，并自动维护 yylineno */
%option noyywrap
%option yylineno
/* —— 规则区：正则模式 + 动作 —— */
%%
"int"                   { return INT; }
"void"                  { return VOID; }

"if"                    { return IF; }
"else"                  { return ELSE; }
"while"                 { return WHILE; }
"break"                 { return BREAK; }
"continue"              { return CONTINUE; }
"return"                { return RETURN; }


[ \t]+               ;   /* 忽略空格、制表符*/
\r?\n                  ;

"//"[^\r\n]*                 ;   /* 忽略 C++ 单行注释 */
\/\*([^*]|\*+[^/*])*\*+\/    ;  /* 忽略 C 风格多行注释 */

[0-9]+                 {
                         /* 匹配整数，存入 yylval 并返回 NUMBER token */
                         yylval.intval = std::atoi(yytext);
                         return NUMBER;
                      }

[A-Za-z_][A-Za-z0-9_]* {
                         /* 匹配标识符，复制文本后返回 IDENTIFIER token */
                         yylval.strval = strdup(yytext);
                         return IDENTIFIER;
                      }

"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return MULTIPLY; }
"/"                     { return DIVIDE; }
"%"                     { return PRECENT; }

"!"                     { return EXCLAPOINT; }
"=="                    { return EQ; // 先声明长运算符,flex自动匹配最长的}    
"<="                    { return LE; }
">="                    { return GE; }
"<"                     { return LT; }
">"                     { return GT; }
"||"                    { return OR; }
"&&"                    { return AND; }

"("                     { return LPAREN; }
")"                     { return RPAREN; }
";"                     { return SEMI; }
"="                     { return ASSIGN; }
"{"                     { return LBRACE; }
"}"                     { return RBRACE; }
","                     { return COMMA; }

.                       {
                         /* 其他任何单字符，报错但不退出 */
                         fprintf(stderr,
                                 "Lexer error at line %d: unexpected '%s'\n",
                                 yylineno, yytext);
                      }

%%

/* —— 用户子程序区 —— */

 /*int yywrap() { return 1; }*/