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
        gen(code[i]);
    }
    return 0;
}