#include <cstdio>

int yylex(void);
extern FILE* yyin;
int main(int argc, char** argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            perror("fopen");
            return 1;
        }
    }
    // 这一步会不断匹配 lexer.l 中的规则，产生 token/动作,输入缓冲区（默认4096字节）
    int i=yylex();      
    return 0;
}
