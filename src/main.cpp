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
    // ��һ���᲻��ƥ�� lexer.l �еĹ��򣬲��� token/����,���뻺������Ĭ��4096�ֽڣ�
    int i=yylex();      
    return 0;
}
