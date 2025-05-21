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

    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    gen(node);

    printf("\tpop rax\n");
    printf("\tret\n");
    return 0;
}