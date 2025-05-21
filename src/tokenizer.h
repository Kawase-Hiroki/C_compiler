#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>

typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_E0F,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

extern Token *token;

void error(char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number(void);
bool ar_eof(void);
Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize(char *p);

# endif