#include <stdio.h>

#include "parse.h"
#include "tokenizer.h"
static int labels, loop_depth, loop_end[256], loop_cont[256];
static char* argregs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
void gen(Node* n);
static int width(Type* t) { return t && t->kind == TY_CHAR ? 1 : t && t->kind == TY_PTR ? 8
                                                                                        : 4; }
static void address(Node* n) {
    if (n->kind == ND_LVAR) {
        if (n->var->is_global)
            printf("\tlea rax, [rip + %s]\n", n->var->name);
        else
            printf("\tlea rax, [rbp - %d]\n", n->offset);
        printf("\tpush rax\n");
        return;
    }
    if (n->kind == ND_DEREF) {
        gen(n->lhs);
        return;
    }
    error("not an lvalue");
}
void gen(Node* n) {
    if (!n)
        return;
    if (n->kind == ND_NUM) {
        printf("\tpush %ld\n", n->val);
        return;
    }
    if (n->kind == ND_STR) {
        printf(".pushsection .rodata\n%s:\n\t.byte ", n->name);
        for (int i = 0; i < n->len; i++) printf("%s%d", i ? "," : "", (unsigned char)n->str[i]);
        printf("%s\n.popsection\n", n->len ? ",0" : "0");
        printf("\tlea rax, [rip + %s]\n\tpush rax\n", n->name);
        return;
    }
    if (n->kind == ND_GVAR) {
        printf("\t.comm %s, %d, %d\n", n->var->name, n->var->ty->size, n->var->ty->size >= 8 ? 8 : n->var->ty->size);
        return;
    }
    if (n->kind == ND_LVAR) {
        if (n->ty->kind == TY_ARRAY) {
            address(n);
            return;
        }
        address(n);
        printf("\tpop rax\n");
        if (width(n->ty) == 1)
            printf("\tmovsxb rax, byte ptr [rax]\n");
        else if (width(n->ty) == 4)
            printf("\tmovsxd rax, dword ptr [rax]\n");
        else
            printf("\tmov rax, [rax]\n");
        printf("\tpush rax\n");
        return;
    }
    if (n->kind == ND_ADDR) {
        address(n->lhs);
        return;
    }
    if (n->kind == ND_DEREF) {
        gen(n->lhs);
        if (n->ty && n->ty->kind == TY_ARRAY)
            return;
        printf("\tpop rax\n");
        if (width(n->ty) == 1)
            printf("\tmovsxb rax, byte ptr [rax]\n");
        else if (width(n->ty) == 4)
            printf("\tmovsxd rax, dword ptr [rax]\n");
        else
            printf("\tmov rax, [rax]\n");
        printf("\tpush rax\n");
        return;
    }
    if (n->kind == ND_ASSIGN) {
        address(n->lhs);
        gen(n->rhs);
        printf("\tpop rdi\n\tpop rax\n");
        if (width(n->lhs->ty) == 1)
            printf("\tmov byte ptr [rax], dil\n");
        else if (width(n->lhs->ty) == 4)
            printf("\tmov dword ptr [rax], edi\n");
        else
            printf("\tmov [rax], rdi\n");
        printf("\tpush rdi\n");
        return;
    }
    if (n->kind == ND_RETURN) {
        gen(n->lhs);
        printf("\tpop rax\n\tmov rsp, rbp\n\tpop rbp\n\tret\n");
        return;
    }
    if (n->kind == ND_IF) {
        int id = ++labels;
        gen(n->cond);
        printf("\tpop rax\n\ttest rax, rax\n\tje .Lelse%d\n", id);
        gen(n->then);
        printf("\tjmp .Lifend%d\n.Lelse%d:\n", id, id);
        gen(n->els);
        printf(".Lifend%d:\n", id);
        return;
    }
    if (n->kind == ND_WHILE || n->kind == ND_FOR) {
        int id = ++labels;
        loop_end[loop_depth] = id;
        loop_cont[loop_depth++] = (n->kind == ND_FOR ? -id : id);
        if (n->kind == ND_FOR)
            gen(n->init);
        printf(".Lbegin%d:\n", id);
        if (n->kind == ND_WHILE)
            gen(n->lhs);
        else
            gen(n->cond);
        if (n->kind == ND_WHILE || n->cond)
            printf("\tpop rax\n\ttest rax, rax\n\tje .Lend%d\n", id);
        gen(n->kind == ND_WHILE ? n->rhs : n->body);
        if (n->kind == ND_FOR) {
            printf(".Lcontinue%d:\n", id);
            gen(n->inc);
        }
        printf("\tjmp .Lbegin%d\n.Lend%d:\n", id, id);
        loop_depth--;
        return;
    }
    if (n->kind == ND_BREAK || n->kind == ND_CONTINUE) {
        if (!loop_depth)
            error("break/continue outside loop");
        if (n->kind == ND_BREAK)
            printf("\tjmp .Lend%d\n", loop_end[loop_depth - 1]);
        else if (loop_cont[loop_depth - 1] < 0)
            printf("\tjmp .Lcontinue%d\n", -loop_cont[loop_depth - 1]);
        else
            printf("\tjmp .Lbegin%d\n", loop_cont[loop_depth - 1]);
        return;
    }
    if (n->kind == ND_BLOCK) {
        for (int i = 0; i < n->stmt_count; i++) gen(n->stmts[i]);
        return;
    }
    if (n->kind == ND_FUNCDEF) {
        printf(".global %s\n%s:\n\tpush rbp\n\tmov rbp, rsp\n\tsub rsp, %d\n", n->name, n->name, (n->offset + 15) / 16 * 16);
        for (int i = 0; i < n->stmt_count; i++) {
            if (i >= 6)
                error("maximum of six parameters supported");
            printf("\tmov [rbp - %d], %s\n", n->stmts[i]->offset, argregs[i]);
        }
        gen(n->body);
        printf("\tmov rsp, rbp\n\tpop rbp\n\tret\n");
        return;
    }
    if (n->kind == ND_CALL) {
        for (int i = 0; i < n->stmt_count; i++) gen(n->stmts[i]);
        for (int i = n->stmt_count - 1; i >= 0; i--) {
            if (i >= 6)
                error("maximum of six arguments supported");
            printf("\tpop %s\n", argregs[i]);
        }
        printf("\txor eax, eax\n\tcall %s\n\tpush rax\n", n->name);
        return;
    }
    if (n->kind == ND_LOGAND || n->kind == ND_LOGOR) {
        int id = ++labels;
        gen(n->lhs);
        printf("\tpop rax\n\ttest rax, rax\n");
        printf(n->kind == ND_LOGAND ? "\tje .Llogic%d\n" : "\tjne .Llogic%d\n", id);
        gen(n->rhs);
        printf("\tpop rax\n\ttest rax, rax\n\tsetne al\n\tmovzx rax, al\n\tjmp .Llogicend%d\n.Llogic%d:\n\tmov rax, %d\n.Llogicend%d:\n\tpush rax\n", id, id, n->kind == ND_LOGOR, id);
        return;
    }
    if (n->kind == ND_NOT) {
        gen(n->lhs);
        printf("\tpop rax\n\ttest rax, rax\n\tsete al\n\tmovzx rax, al\n\tpush rax\n");
        return;
    }
    if (n->kind == ND_NULL)
        return;
    gen(n->lhs);
    gen(n->rhs);
    printf("\tpop rdi\n\tpop rax\n");
    switch (n->kind) {
    case ND_ADD:
        printf("\tadd rax, rdi\n");
        break;
    case ND_SUB:
        printf("\tsub rax, rdi\n");
        break;
    case ND_MUL:
        printf("\timul rax, rdi\n");
        break;
    case ND_DIV:
        printf("\tcqo\n\tidiv rdi\n");
        break;
    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
    case ND_GT:
    case ND_GE:
        printf("\tcmp rax, rdi\n\tset%s al\n\tmovzx rax, al\n", n->kind == ND_EQ ? "e" : n->kind == ND_NE ? "ne"
                                                                                     : n->kind == ND_LT   ? "l"
                                                                                     : n->kind == ND_LE   ? "le"
                                                                                     : n->kind == ND_GT   ? "g"
                                                                                                          : "ge");
        break;
    default:
        error("unsupported expression node");
    }
    printf("\tpush rax\n");
}
