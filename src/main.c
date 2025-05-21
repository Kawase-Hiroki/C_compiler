#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "generator.h"
#include "parse.h"
#include "tokenizer.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("incrrect number of arguments\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(user_input);
    program();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    printf("\tpush rbp\n");
    printf("\tmov rbp, rsp\n");
    printf("\tsub rsp, 208\n");

    for (int i = 0; code[i]; i++) {
        gen(code[i]);
        printf("\tpop rax\n");
    }

    printf("\tmov rsp, rbp\n");
    printf("\tpop rbp\n");
    printf("\tret\n");
    return 0;
}