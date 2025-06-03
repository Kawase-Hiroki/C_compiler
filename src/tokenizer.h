#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>

typedef enum {
    TK_RESERVED,
    TK_IDENT,
    TK_NUM,
    TK_E0F,
    TK_RETURN,
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

typedef struct LVar LVar;

struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
};

extern LVar *locals;

void error(char *fmt, ...);
bool consume(char *op); 
void expect(char *op);
Token *consume_ident(void);
int expect_number(void);
bool at_eof(void);
int is_alnum(char c);
LVar *find_lvar(Token *tok);
Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize(char *p);

# endif