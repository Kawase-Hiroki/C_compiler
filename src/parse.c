#include "parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
Type* type_int(void) {
    Type* t = calloc(1, sizeof(*t));
    t->kind = TY_INT;
    t->size = 4;
    return t;
}
Type* type_char(void) {
    Type* t = calloc(1, sizeof(*t));
    t->kind = TY_CHAR;
    t->size = 1;
    return t;
}
Type* pointer_to(Type* b) {
    Type* t = calloc(1, sizeof(*t));
    t->kind = TY_PTR;
    t->base = b;
    t->size = 8;
    return t;
}
Type* array_of(Type* b, int n) {
    Type* t = calloc(1, sizeof(*t));
    t->kind = TY_ARRAY;
    t->base = b;
    t->array_len = n;
    t->size = b->size * n;
    return t;
}
Node* code[256];
static int code_n, label_n;
static Node* node(NodeKind k, Node* a, Node* b) {
    Node* n = calloc(1, sizeof(*n));
    n->kind = k;
    n->lhs = a;
    n->rhs = b;
    return n;
}
static Node* num(long v) {
    Node* n = node(ND_NUM, 0, 0);
    n->val = v;
    n->ty = type_int();
    return n;
}
static bool type_start(void) { return token->kind == TK_RESERVED && ((token->len == 3 && !strncmp(token->str, "int", 3)) || (token->len == 4 && !strncmp(token->str, "char", 4))); }
static Type* base_type(void) {
    Type* t;
    if (token->len == 3)
        t = type_int();
    else
        t = type_char();
    token = token->next;
    while (consume("*")) t = pointer_to(t);
    return t;
}
static LVar* add_var(Token* t, Type* ty, bool glob) {
    LVar* v = calloc(1, sizeof(*v));
    v->name = strndup(t->str, t->len);
    v->len = t->len;
    v->ty = ty;
    v->is_global = glob;
    v->next = glob ? globals : locals;
    if (!glob)
        v->offset = (locals ? locals->offset : 0) + ((ty->size + 7) / 8) * 8;
    if (glob)
        globals = v;
    else
        locals = v;
    return v;
}
static Node* expr(void);
static Node* stmt(void);
static Node* assign(void);
static Node* primary(void);
static Node* logical_and(void);
static Node* equality(void);
static Node* relational(void);
static Node* add(void);
static Node* mul(void);
static Node* unary(void);
static Node* lvar_node(LVar* v) {
    Node* n = node(ND_LVAR, 0, 0);
    n->var = v;
    n->ty = v->ty;
    n->offset = v->offset;
    return n;
}
static Node* logical_or(void) {
    Node* n = logical_and();
    while (consume("||")) n = node(ND_LOGOR, n, logical_and());
    return n;
}
static Node* logical_and(void) {
    Node* n = equality();
    while (consume("&&")) n = node(ND_LOGAND, n, equality());
    return n;
}
static Node* equality(void) {
    Node* n = relational();
    for (;;) {
        if (consume("=="))
            n = node(ND_EQ, n, relational());
        else if (consume("!="))
            n = node(ND_NE, n, relational());
        else
            return n;
    }
}
static Node* relational(void) {
    Node* n = add();
    for (;;) {
        if (consume("<="))
            n = node(ND_LE, n, add());
        else if (consume("<"))
            n = node(ND_LT, n, add());
        else if (consume(">="))
            n = node(ND_GE, n, add());
        else if (consume(">"))
            n = node(ND_GT, n, add());
        else
            return n;
    }
}
static Node* add(void) {
    Node* n = mul();
    for (;;) {
        if (consume("+")) {
            Node* r = mul();
            if (n->ty && (n->ty->kind == TY_PTR || n->ty->kind == TY_ARRAY)) {
                if (r->kind == ND_NUM)
                    r->val *= n->ty->base->size;
                else {
                    Node* m = node(ND_MUL, r, num(n->ty->base->size));
                    r = m;
                }
            } else if (r->ty && r->ty->kind == TY_PTR) {
                if (n->kind == ND_NUM)
                    n->val *= r->ty->base->size;
                else
                    n = node(ND_MUL, n, num(r->ty->base->size));
            }
            n = node(ND_ADD, n, r);
            n->ty = (n->lhs->ty && (n->lhs->ty->kind == TY_PTR || n->lhs->ty->kind == TY_ARRAY)) ? (n->lhs->ty->kind == TY_ARRAY ? pointer_to(n->lhs->ty->base) : n->lhs->ty) : n->rhs->ty;
        } else if (consume("-")) {
            Node* r = mul();
            bool ptrdiff = n->ty && n->ty->kind == TY_PTR && r->ty && r->ty->kind == TY_PTR;
            int scale = ptrdiff ? n->ty->base->size : 1;
            if (n->ty && (n->ty->kind == TY_PTR || n->ty->kind == TY_ARRAY) && !ptrdiff) {
                if (r->kind == ND_NUM)
                    r->val *= scale;
                else
                    r = node(ND_MUL, r, num(scale));
            }
            n = node(ND_SUB, n, r);
            n->ty = ptrdiff ? type_int() : n->lhs->ty;
            if (ptrdiff && scale > 1)
                n = node(ND_DIV, n, num(scale));
        } else
            return n;
    }
}
static Node* mul(void) {
    Node* n = unary();
    for (;;) {
        if (consume("*"))
            n = node(ND_MUL, n, unary());
        else if (consume("/"))
            n = node(ND_DIV, n, unary());
        else
            return n;
    }
}
static Node* unary(void) {
    if (consume("+"))
        return unary();
    if (consume("-"))
        return node(ND_SUB, num(0), unary());
    if (consume("!"))
        return node(ND_NOT, unary(), 0);
    if (consume("&")) {
        Node* n = node(ND_ADDR, unary(), 0);
        n->ty = pointer_to(n->lhs->ty);
        return n;
    }
    if (consume("*")) {
        Node* n = node(ND_DEREF, unary(), 0);
        n->ty = n->lhs->ty && n->lhs->ty->kind == TY_PTR ? n->lhs->ty->base : type_int();
        return n;
    }
    if (token->kind == TK_SIZEOF) {
        token = token->next;
        if (consume("(")) {
            if (type_start()) {
                Type* t = base_type();
                if (consume("[")) {
                    int len = (int)expect_number();
                    expect("]");
                    t = array_of(t, len);
                }
                expect(")");
                return num(t->size);
            }
            Node* n = expr();
            expect(")");
            return num(n->ty ? n->ty->size : 8);
        }
        Node* n = unary();
        return num(n->ty ? n->ty->size : 8);
    }
    return primary();
}
static Node* primary(void) {
    if (consume("(")) {
        Node* n = expr();
        expect(")");
        return n;
    }
    if (token->kind == TK_NUM || token->kind == TK_CHARLIT) {
        long v = token->val;
        token = token->next;
        return num(v);
    }
    if (token->kind == TK_STR) {
        Node* n = node(ND_STR, 0, 0);
        n->ty = pointer_to(type_char());
        n->name = malloc(32);
        sprintf(n->name, ".Lstr%d", label_n++);
        n->str = token->contents;
        n->len = token->contents_len;
        token = token->next;
        while (consume("[")) {
            Node* i = expr();
            expect("]");
            Node* a = node(ND_ADD, n, i);
            a->ty = n->ty;
            Node* d = node(ND_DEREF, a, 0);
            d->ty = type_char();
            n = d;
        }
        return n;
    }
    Token* t = consume_ident();
    if (!t)
        error("expected expression");
    if (consume("(")) {
        Node* n = node(ND_CALL, 0, 0);
        n->name = strndup(t->str, t->len);
        if (!consume(")")) {
            for (;;) {
                Node* a = assign();
                n->stmts = realloc(n->stmts, sizeof(Node*) * (n->stmt_count + 1));
                n->stmts[n->stmt_count++] = a;
                if (consume(")"))
                    break;
                expect(",");
            }
        }
        n->ty = type_int();
        return n;
    }
    LVar* v = find_lvar(t);
    if (!v)
        error("use of undeclared identifier: %.*s", t->len, t->str);
    Node* n = lvar_node(v);
    while (consume("[")) {
        Node* i = expr();
        expect("]");
        Type* base = n->ty->kind == TY_ARRAY ? n->ty->base : n->ty->base;
        if (base->size != 1)
            i = node(ND_MUL, i, num(base->size));
        Node* a = node(ND_ADD, n, i);
        a->ty = pointer_to(base);
        Node* d = node(ND_DEREF, a, 0);
        d->ty = a->ty->base;
        n = d;
    }
    return n;
}
static Node* assign(void) {
    Node* n = logical_or();
    if (consume("=")) {
        Node* r = assign();
        return node(ND_ASSIGN, n, r);
    }
    return n;
}
static Node* expr(void) { return assign(); }
static Node* declaration(void) {
    Type* t = base_type();
    Token* id = consume_ident();
    if (!id)
        error("expected variable name");
    while (consume("[")) {
        int len = expect_number();
        expect("]");
        t = array_of(t, (int)len);
    }
    LVar* v = add_var(id, t, false);
    if (consume("=")) {
        Node* n = node(ND_ASSIGN, lvar_node(v), expr());
        expect(";");
        return n;
    }
    expect(";");
    return node(ND_NULL, 0, 0);
}
static Node* stmt(void) {
    if (type_start())
        return declaration();
    if (token->kind == TK_BREAK || token->kind == TK_CONTINUE) {
        Node* n = node(token->kind == TK_BREAK ? ND_BREAK : ND_CONTINUE, 0, 0);
        token = token->next;
        expect(";");
        return n;
    }
    if (token->kind == TK_RETURN) {
        token = token->next;
        Node* n = node(ND_RETURN, expr(), 0);
        expect(";");
        return n;
    }
    if (token->kind == TK_IF) {
        token = token->next;
        expect("(");
        Node* n = node(ND_IF, 0, 0);
        n->cond = expr();
        expect(")");
        n->then = stmt();
        if (token->kind == TK_ELSE) {
            token = token->next;
            n->els = stmt();
        }
        return n;
    }
    if (token->kind == TK_WHILE) {
        token = token->next;
        expect("(");
        Node* n = node(ND_WHILE, 0, 0);
        n->lhs = expr();
        expect(")");
        n->rhs = stmt();
        return n;
    }
    if (token->kind == TK_FOR) {
        token = token->next;
        expect("(");
        Node* n = node(ND_FOR, 0, 0);
        if (!consume(";")) {
            n->init = expr();
            expect(";");
        }
        if (!consume(";")) {
            n->cond = expr();
            expect(";");
        }
        if (!consume(")")) {
            n->inc = expr();
            expect(")");
        }
        n->body = stmt();
        return n;
    }
    if (consume("{")) {
        Node* n = node(ND_BLOCK, 0, 0);
        while (!consume("}")) {
            if (at_eof())
                error("unterminated block");
            n->stmts = realloc(n->stmts, sizeof(Node*) * (n->stmt_count + 1));
            n->stmts[n->stmt_count++] = stmt();
        }
        return n;
    }
    Node* n = expr();
    expect(";");
    return n;
}
static Node* function(Type* ret, Token* name) {
    locals = NULL;
    expect("(");
    Node* n = node(ND_FUNCDEF, 0, 0);
    n->name = strndup(name->str, name->len);
    n->ty = ret;
    if (!consume(")")) {
        do {
            Type* t = base_type();
            Token* a = consume_ident();
            if (!a)
                error("expected parameter name");
            LVar* v = add_var(a, t, false);
            Node* p = lvar_node(v);
            n->stmts = realloc(n->stmts, sizeof(Node*) * (n->stmt_count + 1));
            n->stmts[n->stmt_count++] = p;
        } while (consume(","));
        expect(")");
    }
    n->body = stmt();
    n->offset = locals ? locals->offset : 0;
    return n;
}
void program(void) {
    while (!at_eof()) {
        if (!type_start())
            error("expected declaration");
        Type* t = base_type();
        Token* id = consume_ident();
        if (!id)
            error("expected identifier");
        if (token->kind == TK_RESERVED && token->len == 1 && *token->str == '(') {
            code[code_n++] = function(t, id);
            continue;
        }
        while (consume("[")) {
            int len = expect_number();
            expect("]");
            t = array_of(t, (int)len);
        }
        LVar* v = add_var(id, t, true);
        Node* n = node(ND_GVAR, 0, 0);
        n->var = v;
        code[code_n++] = n;
        expect(";");
    }
    code[code_n] = NULL;
}
int locals_size(void) {
    int n = locals ? locals->offset : 0;
    return (n + 15) / 16 * 16;
}
