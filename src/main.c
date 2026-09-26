#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "parse.h"
#include "tokenizer.h"

static char* read_file(const char* path) {
    FILE* fp = fopen(path, "rb");
    if (!fp)
        return NULL;
    if (fseek(fp, 0, SEEK_END)) {
        fclose(fp);
        return NULL;
    }
    long n = ftell(fp);
    rewind(fp);
    char* buf = calloc((size_t)n + 1, 1);
    if (fread(buf, 1, n, fp) != (size_t)n) {
        fclose(fp);
        free(buf);
        return NULL;
    }
    fclose(fp);
    return buf;
}
int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <file|source>\n", argv[0]);
        return 1;
    }
    user_input = read_file(argv[1]);
    if (!user_input)
        user_input = argv[1];
    token = tokenize(user_input);
    program();
    printf(".intel_syntax noprefix\n");
    for (int i = 0; code[i]; i++) gen(code[i]);
    printf(".section .note.GNU-stack,\"\",@progbits\n");
    return 0;
}
