#include "tokenizer.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Token *token;
LVar *locals;

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char *op) {
    if (token->kind != TK_RESERVED || strncmp(token->str, op, strlen(op)) != 0) {
        return false;
    }
    token = token->next;
    return true;
}

Token *consume_ident() {
    if (token->kind != TK_IDENT) {
        return NULL;
    }
    Token *t = token;
    token = token->next;
    return t;
}

void expect(char *op) {
    if (token->kind != TK_RESERVED || strncmp(token->str, op, strlen(op))) {
        error("not %c", op);
    }
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM) {
        error("not number");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_E0F;
}

int is_alnum(char c){
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '_';
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next) {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
            return var;
        }
    }
    return NULL;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (!strncmp(p, "!=", 2) || !strncmp(p, "==", 2) ||
            !strncmp(p, "<=", 2) || !strncmp(p, ">=", 2)) {
            cur = new_token(TK_RESERVED, cur, p);
            p += 2;
            continue;
        }

        if (strchr("+-*/()<>=;", *p)) {
            cur = new_token(TK_RESERVED, cur, p);
            p++;
            continue;
        }

        if(!strncmp(p, "return", 6) && !is_alnum(p[6])){
            cur = new_token(TK_RETURN, cur, p);
            p += 6;
            continue;
        }

        if (('a' <= *p && *p <= 'z') || ('A' <= *p && *p <= 'Z') || *p == '_') {
            char *start = p;
            while (('a' <= *p && *p <= 'z') ||
                   ('A' <= *p && *p <= 'Z') ||
                   ('0' <= *p && *p <= '9') ||
                   *p == '_') {
                p++;
            }
            cur = new_token(TK_IDENT, cur, start);
            cur->len = p - start;
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("cant tokenize");
    }

    new_token(TK_E0F, cur, p);
    return head.next;
}