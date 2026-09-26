#ifndef TOKENIZER_H
#define TOKENIZER_H
#include <stdbool.h>
typedef enum { TK_RESERVED,
               TK_IDENT,
               TK_NUM,
               TK_EOF,
               TK_RETURN,
               TK_IF,
               TK_ELSE,
               TK_WHILE,
               TK_FOR,
               TK_BREAK,
               TK_CONTINUE,
               TK_STR,
               TK_CHARLIT,
               TK_SIZEOF } TokenKind;
typedef struct Token Token;
struct Token {
    TokenKind kind;
    Token* next;
    long val;
    char* str;
    int len;
    char* contents;
    int contents_len;
};
extern Token* token;
extern char* user_input;
typedef struct Type Type;
typedef enum { TY_INT,
               TY_CHAR,
               TY_PTR,
               TY_ARRAY } TypeKind;
struct Type {
    TypeKind kind;
    Type* base;
    int array_len;
    int size;
};
typedef struct LVar LVar;
struct LVar {
    LVar* next;
    char* name;
    int len;
    int offset;
    Type* ty;
    bool is_global;
};
extern LVar *locals, *globals;
void error(char* fmt, ...);
bool consume(char* op);
void expect(char* op);
Token* consume_ident(void);
long expect_number(void);
bool at_eof(void);
int is_alnum(char c);
LVar* find_lvar(Token* tok);
Token* tokenize(char* p);
#endif
