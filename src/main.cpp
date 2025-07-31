// src/main.cpp
#include <cstdio>
#include <iostream>
#include "ast.hpp"          // 使用 Program* 类型

int yylex(void);
extern Program* g_root;// g_root 是指向 AST 根节点的全局指针，在 parser.y 中定义 
extern FILE* yyin;// yyin 是词法分析器的输入文件流，在 lexer.l 中定义
extern int yyparse(void);// yyparse() 是由 Bison 从 parser.y 生成的语法分析主函数
int main(int argc, char** argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            perror("fopen");
            return 1;
        }
    }
    //// 这一步会不断匹配 lexer.l 中的规则，产生 token/动作,输入缓冲区（默认4096字节）
    //int i=yylex();    
    // 调用 yyparse() 启动完整的词法和语法分析过程。
    // yyparse() 会返回 0 表示成功，非 0 表示失败。
    int parse_result = yyparse();

    // 检查分析结果
    if (parse_result == 0 && g_root != nullptr) {
        printf("Parsing successful!\n");
        printf("AST root has been created.\n");
        // 在这里，你可以开始下一阶段，例如遍历 AST 进行语义分析或代码生成
        // 例如：
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
