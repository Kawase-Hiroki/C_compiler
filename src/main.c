#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    for (int i = 0; code[i]; i++) {
        if (strcmp(code[i]->funcname, "main") == 0) {
            printf(".global main\n");
            printf("main:\n");
            printf("\tpush rbp\n");
            printf("\tmov rbp, rsp\n");
            printf("\tsub rsp, 208\n");
        } else {
            printf(".global %s\n", code[i]->funcname);
            printf("%s:\n", code[i]->funcname);
            printf("\tpush rbp\n");
            printf("\tmov rbp, rsp\n");
            printf("\tsub rsp, 208\n");
        }

        gen(code[i]->body);

        printf("\tmov rsp, rbp\n");
        printf("\tpop rbp\n");
        printf("\tret\n");
    }
    return 0;
}