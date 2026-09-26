#ifndef PARSE_H
#define PARSE_H
#include "tokenizer.h"
typedef enum { ND_ADD,
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
               ND_BLOCK,
               ND_CALL,
               ND_FUNCDEF,
               ND_ADDR,
               ND_DEREF,
               ND_NULL,
               ND_LOGAND,
               ND_LOGOR,
               ND_NOT,
               ND_BREAK,
               ND_CONTINUE,
               ND_GVAR,
               ND_STR,
               ND_SIZEOF } NodeKind;
typedef struct Node Node;
struct Node {
    NodeKind kind;
    Node *lhs, *rhs, *then, *els, *init, *cond, *inc, *body;
    long val;
    int offset;
    char* name;
    char* str;
    int len;
    Type* ty;
    LVar* var;
    Node** stmts;
    int stmt_count;
};
extern Node* code[256];
Type* type_int(void);
Type* type_char(void);
Type* pointer_to(Type* base);
Type* array_of(Type* base, int len);
int locals_size(void);
void program(void);
void gen(Node* node);
void error(char* fmt, ...);
#endif
