// src/main.cpp
#include <cstdio>
#include <iostream>
#include "ast.hpp"          // ʹ�� Program* ����

int yylex(void);
extern Program* g_root;// g_root ��ָ�� AST ���ڵ��ȫ��ָ�룬�� parser.y �ж��� 
extern FILE* yyin;// yyin �Ǵʷ��������������ļ������� lexer.l �ж���
extern int yyparse(void);// yyparse() ���� Bison �� parser.y ���ɵ��﷨����������
int main(int argc, char** argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            perror("fopen");
            return 1;
        }
    }
    //// ��һ���᲻��ƥ�� lexer.l �еĹ��򣬲��� token/����,���뻺������Ĭ��4096�ֽڣ�
    //int i=yylex();    
    // ���� yyparse() ���������Ĵʷ����﷨�������̡�
    // yyparse() �᷵�� 0 ��ʾ�ɹ����� 0 ��ʾʧ�ܡ�
    int parse_result = yyparse();

    // ���������
    if (parse_result == 0 && g_root != nullptr) {
        printf("Parsing successful!\n");
        printf("AST root has been created.\n");
        // ���������Կ�ʼ��һ�׶Σ�������� AST ��������������������
        // ���磺
        // SemanticAnalyzer analyzer;
        // analyzer.analyze(g_root);
    }
    else {
        printf("Parsing failed.\n");
    }

    if (yyin) {
        fclose(yyin);
    }

    return parse_result;
    return 0;
}
