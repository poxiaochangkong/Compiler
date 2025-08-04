// src/main.cpp
#include <cstdio>
#include <iostream>
#include "ast.hpp"          // 使用 Program* 类型
#include "SemanticAnalyzer.hpp"//语义分析
// Bison 调试开关 (yydebug)
// 在 parser.y 编译时加入 -t 标志
extern int yydebug;

// Flex 调试开关 (yy_flex_debug)
extern int yy_flex_debug;

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
    yydebug = 1;        // 开启 Bison 语法分析调试信息
    yy_flex_debug = 1;  // 开启 Flex 词法分析调试信息

    //// 这一步会不断匹配 lexer.l 中的规则，产生 token/动作,输入缓冲区（默认4096字节）
    //int i=yylex();    
    // 调用 yyparse() 启动完整的词法和语法分析过程。
    // yyparse() 会返回 0 表示成功，非 0 表示失败。
    int parse_result = yyparse();

    // 检查分析结果
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
