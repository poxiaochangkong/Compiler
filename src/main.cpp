// src/main.cpp
#include <cstdio>
#include <iostream>
#include "ast.hpp"          // ʹ�� Program* ����
#include "SemanticAnalyzer.hpp"//�������
// Bison ���Կ��� (yydebug)
// �� parser.y ����ʱ���� -t ��־
extern int yydebug;

// Flex ���Կ��� (yy_flex_debug)
extern int yy_flex_debug;

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
    yydebug = 1;        // ���� Bison �﷨����������Ϣ
    yy_flex_debug = 1;  // ���� Flex �ʷ�����������Ϣ

    //// ��һ���᲻��ƥ�� lexer.l �еĹ��򣬲��� token/����,���뻺������Ĭ��4096�ֽڣ�
    //int i=yylex();    
    // ���� yyparse() ���������Ĵʷ����﷨�������̡�
    // yyparse() �᷵�� 0 ��ʾ�ɹ����� 0 ��ʾʧ�ܡ�
    int parse_result = yyparse();

    // ���������
    if (parse_result == 0 && g_root != nullptr) {
        std::cout << "\nParsing successful! AST created." << std::endl;

        std::cout << "--- Starting Semantic Analysis ---" << std::endl;
        SemanticAnalyzer analyzer;
        analyzer.analyze(g_root);
        std::cout << "--- Semantic Analysis Finished ---" << std::endl;

    }
    else {
        std::cout << "\nParsing failed." << std::endl;
    }

    if (yyin) {
        fclose(yyin);
    }

    return parse_result;
}
