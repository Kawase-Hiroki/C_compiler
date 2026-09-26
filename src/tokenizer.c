#include "tokenizer.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
Token* token;
LVar *locals, *globals;
void error(char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    if (token && user_input && token->str >= user_input && token->str <= user_input + strlen(user_input)) {
        char *loc = token->str, *begin = loc, *end = loc;
        while (begin > user_input && begin[-1] != '\n') begin--;
        while (*end && *end != '\n') end++;
        fwrite(begin, 1, (size_t)(end - begin), stderr);
        fputc('\n', stderr);
        for (char* p = begin; p < loc; p++) fputc(*p == '\t' ? '\t' : ' ', stderr);
        fputs("^ ", stderr);
    }
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(1);
}
bool consume(char* op) {
    size_t n = strlen(op);
    if (token->kind != TK_RESERVED || token->len != (int)n || strncmp(token->str, op, n))
        return false;
    token = token->next;
    return true;
}
Token* consume_ident(void) {
    if (token->kind != TK_IDENT)
        return NULL;
    Token* t = token;
    token = token->next;
    return t;
}
void expect(char* op) {
    if (!consume(op))
        error("expected '%s'", op);
}
long expect_number(void) {
    if (token->kind != TK_NUM)
        error("expected number");
    long v = token->val;
    token = token->next;
    return v;
}
bool at_eof(void) { return token->kind == TK_EOF; }
int is_alnum(char c) { return isalnum((unsigned char)c) || c == '_'; }
static Token* new_token(Token* cur, TokenKind k, char* s, int n) {
    Token* t = calloc(1, sizeof(*t));
    t->kind = k;
    t->str = s;
    t->len = n;
    cur->next = t;
    return t;
}
static int unescape(char c) {
    switch (c) {
    case 'n':
        return '\n';
    case 't':
        return '\t';
    case 'r':
        return '\r';
    case '0':
        return 0;
    case '\\':
        return '\\';
    case '"':
        return '"';
    case '\'':
        return '\'';
    default:
        return c;
    }
}
Token* tokenize(char* p) {
    Token head = {0}, *cur = &head;
    while (*p) {
        if (isspace((unsigned char)*p)) {
            p++;
            continue;
        }
        if (!strncmp(p, "//", 2)) {
            p += 2;
            while (*p && *p != '\n') p++;
            continue;
        }
        if (!strncmp(p, "/*", 2)) {
            p += 2;
            while (*p && strncmp(p, "*/", 2)) p++;
            if (!*p)
                error("unterminated block comment");
            p += 2;
            continue;
        }
        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            char* buf = calloc(strlen(p) + 1, 1);
            int n = 0;
            while (*p && *p != quote) {
                if (*p == '\\') {
                    p++;
                    if (!*p)
                        error("unterminated escape");
                    buf[n++] = unescape(*p++);
                } else
                    buf[n++] = *p++;
            }
            if (!*p)
                error("unterminated literal");
            p++;
            Token* t = new_token(cur, quote == '"' ? TK_STR : TK_CHARLIT, p - n - 1, n);
            t->contents = buf;
            t->contents_len = n;
            cur = t;
            if (quote == '\'' && n != 1)
                error("character literal must contain one character");
            if (quote == '\'')
                t->val = (unsigned char)buf[0];
            continue;
        }
        if (!strncmp(p, "!=", 2) || !strncmp(p, "==", 2) || !strncmp(p, "<=", 2) || !strncmp(p, ">=", 2) || !strncmp(p, "&&", 2) || !strncmp(p, "||", 2)) {
            cur = new_token(cur, TK_RESERVED, p, 2);
            p += 2;
            continue;
        }
        if (strchr("+-*/()<>=;{}&,![]", *p)) {
            cur = new_token(cur, TK_RESERVED, p, 1);
            p++;
            continue;
        }
        struct {
            char* s;
            TokenKind k;
        } kws[] = {{"return", TK_RETURN}, {"if", TK_IF}, {"else", TK_ELSE}, {"while", TK_WHILE}, {"for", TK_FOR}, {"break", TK_BREAK}, {"continue", TK_CONTINUE}, {"sizeof", TK_SIZEOF}};
        int found = 0;
        for (unsigned i = 0; i < sizeof(kws) / sizeof(*kws); i++) {
            int n = strlen(kws[i].s);
            if (!strncmp(p, kws[i].s, n) && !is_alnum(p[n])) {
                cur = new_token(cur, kws[i].k, p, n);
                p += n;
                found = 1;
                break;
            }
        }
        if (found)
            continue;
        if (isalpha((unsigned char)*p) || *p == '_') {
            char* s = p;
            while (is_alnum(*p)) p++;
            int n = p - s;
            cur = new_token(cur, TK_RESERVED, s, n);
            if (!((n == 3 && !strncmp(s, "int", 3)) || (n == 4 && !strncmp(s, "char", 4))))
                cur->kind = TK_IDENT;
            continue;
        }
        if (isdigit((unsigned char)*p)) {
            char* s = p;
            long v = strtol(p, &p, 0);
            cur = new_token(cur, TK_NUM, s, p - s);
            cur->val = v;
            continue;
        }
        error("cannot tokenize near: %.16s", p);
    }
    new_token(cur, TK_EOF, p, 0);
    return head.next;
}
LVar* find_lvar(Token* t) {
    for (LVar* v = locals; v; v = v->next)
        if (v->len == t->len && !memcmp(t->str, v->name, v->len))
            return v;
    for (LVar* v = globals; v; v = v->next)
        if (v->len == t->len && !memcmp(t->str, v->name, v->len))
            return v;
    return NULL;
}
