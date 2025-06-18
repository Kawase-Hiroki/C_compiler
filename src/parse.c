
#include "parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "tokenizer.h"

int locals_size() {
    int size = 0;
    for (LVar *var = locals; var; var = var->next) {
        size += 8;
    }
    if (size % 16 != 0) {
        size = ((size / 16) + 1) * 16;
    }
    return size;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *code[100];

void program() {
    int i = 0;
    while (!at_eof()) {
        code[i++] = function();
    }
    code[i] = NULL;
}

Node *function() {
    locals = NULL;

    Token *tok = consume_ident();
    if (!tok)
        error("need function name");

    char *name = strndup(tok->str, tok->len);

    expect("(");

    Node **params = NULL;
    int param_count = 0;

    if (!consume(")")) {
        while (1) {
            Token *argtok = consume_ident();
            if (!argtok)
                error("expected argument name");

            Node *arg = calloc(1, sizeof(Node));
            arg->kind = ND_LVAR;
            arg->varname = strndup(argtok->str, argtok->len);

            LVar *lvar = calloc(1, sizeof(LVar));
            lvar->name = strndup(argtok->str, argtok->len);
            lvar->len = argtok->len;

            if (locals == NULL) {
                lvar->offset = 8;
            } else {
                lvar->offset = locals->offset + 8;
            }
            lvar->next = locals;
            locals = lvar;

            arg->offset = lvar->offset;

            params = realloc(params, sizeof(Node *) * (param_count + 1));
            params[param_count++] = arg;

            if (consume(")"))
                break;
            expect(",");
        }
    }

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNCDEF;
    node->funcname = name;

    node->stmts = params;
    node->stmt_count = param_count;

    node->body = stmt();
    return node;
}

Node *stmt() {
    if (token->kind == TK_RETURN) {
        token = token->next;
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        expect(";");
        return node;
    }

    if (token->kind == TK_IF) {
        token = token->next;
        expect("(");
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if (consume("else")) {
            node->els = stmt();
        }
        return node;
    }

    if (token->kind == TK_WHILE) {
        token = token->next;
        expect("(");
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        node->lhs = expr();
        expect(")");
        node->rhs = stmt();
        return node;
    }

    if (token->kind == TK_FOR) {
        token = token->next;
        expect("(");
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;

        if (!consume(";")) {
            node->init = expr();
            expect(";");
        }
        if (!consume(";")) {
            node->cond = expr();
            expect(";");
        }
        if (!consume(")")) {
            node->inc = expr();
            expect(")");
        }
        node->body = stmt();
        return node;
    }

    if (consume("{")) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;

        Node **stmts = NULL;
        int cap = 0;
        int len = 0;
        while (!consume("}")) {
            if (len == cap) {
                cap = cap ? cap * 2 : 4;
                stmts = realloc(stmts, sizeof(Node *) * cap);
            }
            stmts[len++] = stmt();
        }
        node->stmts = stmts;
        node->stmt_count = len;
        return node;
    }

    Node *node = expr();
    expect(";");
    return node;
}

Node *expr() {
    Node *node = assign();
    return node;
}

Node *assign() {
    Node *node = equality();
    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *equality() {
    Node *node = relational();
    for (;;) {
        if (consume("==")) {
            node = new_node(ND_EQ, node, relational());
        } else if (consume("!=")) {
            node = new_node(ND_NE, node, relational());
        } else {
            return node;
        }
    }
}

Node *relational() {
    Node *node = add();
    for (;;) {
        if (consume("<=")) {
            node = new_node(ND_LE, node, add());
        } else if (consume("<")) {
            node = new_node(ND_LT, node, add());
        } else if (consume(">=")) {
            node = new_node(ND_GE, node, add());
        } else if (consume(">")) {
            node = new_node(ND_GT, node, add());
        } else {
            return node;
        }
    }
}

Node *add() {
    Node *node = mul();
    for (;;) {
        if (consume("+")) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = unary();
    for (;;) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

Node *unary() {
    if (consume("+")) {
        return unary();
    } else if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), unary());
    } else if (consume("&")) {
        return new_node(ND_ADDR, unary(), NULL);
    } else if (consume("*")) {
        return new_node(ND_DEREF, unary(), NULL);
    }
    return primary();
}

Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }
    Token *tok = consume_ident();

    if (tok) {
        if (consume("(")) {
            Node **args = NULL;
            int arg_count = 0;
            if (!consume(")")) {
                while (1) {
                    args = realloc(args, sizeof(Node *) * (arg_count + 1));
                    args[arg_count++] = assign();
                    if (consume(")"))
                        break;
                    expect(",");
                }
            } else {
            }

            Node *node = calloc(1, sizeof(Node));
            node->kind = ND_CALL;
            node->funcname = strndup(tok->str, tok->len);
            node->stmts = args;
            node->stmt_count = arg_count;
            return node;
        }

        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(tok);
        if (lvar) {
            node->offset = lvar->offset;
        } else {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = strndup(tok->str, tok->len);
            lvar->len = tok->len;
            if (locals) {
                lvar->offset = locals->offset + 8;
            } else {
                lvar->offset = 8;
            }
            node->offset = lvar->offset;
            locals = lvar;
        }
        return node;
    }
    return new_node_num(expect_number());
}