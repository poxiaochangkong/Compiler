#include <cstdio>
#ifdef __cplusplus
extern "C" {
#endif

    int yylex(void);
    extern FILE* yyin;

#ifdef __cplusplus
}
#endif

int main(int argc, char** argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            perror("fopen");
            return 1;
        }
    }
    // ��һ���᲻��ƥ�� lexer.l �еĹ��򣬲��� token/����
    yylex();
    return 0;
}
