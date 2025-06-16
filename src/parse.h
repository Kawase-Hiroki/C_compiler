#ifndef PARSE_H
#define PARSE_H

#include "tokenizer.h"


typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
    ND_LT,
    ND_GT,
    ND_LE,
    ND_GE,
    ND_EQ,
    ND_NE,
    ND_ASSIGN,
    ND_LVAR,
    ND_RETURN,
    ND_IF,
    ND_WHILE,
    ND_FOR,
    ND_ELSE,
    ND_BLOCK,
    ND_CALL,
    ND_FUNCDEF,
    ND_ADDR,
    ND_DEREF,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
    int offset;

    char *funcname;

    Node *then;
    Node *els;
    Node *init;
    Node *cond;
    Node *inc;
    Node *body;

    Node **stmts;
    int stmt_count;
};

typedef struct {
    char *name;
    Node *body;
    LVar *locals;
} Function;

extern Node *code[100];
int locals_size();
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *function(void);
void program();
Node *stmt(void);
Node *expr(void);
Node *equality(void);
Node *assign(void);
Node *relational(void);
Node *add(void);
Node *mul(void);
Node *unary(void);
Node *primary(void);

#endif