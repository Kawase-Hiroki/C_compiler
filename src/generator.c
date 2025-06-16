
#include "generator.h"

#include <stdio.h>

#include "tokenizer.h"

int count = 0;

void gen_lval(Node *node) {
    switch (node->kind) {
    case ND_LVAR:
        printf("\tmov rax, rbp\n");
        printf("\tsub rax, %d\n", node->offset);
        printf("\tpush rax\n");
        return;
    case ND_DEREF:
        gen(node->lhs);
        return;
    default:
        error("lvalue required as left operand of assignment");
    }
}

void gen(Node *node) {
    if (node->kind == ND_RETURN) {
        gen(node->lhs);
        printf("\tpop rax\n");
        printf("\tmov rsp, rbp\n");
        printf("\tpop rbp\n");
        printf("\tret\n");
        return;
    }

    if (node->kind == ND_IF) {
        count++;
        gen(node->cond);
        printf("\tpop rax\n");
        printf("\tcmp rax, 0\n");
        printf("\tje .Lelse%d\n", count);
        gen(node->then);
        printf("\tjmp .Lend%d\n", count);
        printf(".Lelse%d:\n", count);
        if (node->els) {
            gen(node->els);
        }
        printf(".Lend%d:\n", count);
        return;
    }

    if (node->kind == ND_WHILE) {
        count++;
        printf(".Lbegin%d:\n", count);
        gen(node->lhs);
        printf("\tpop rax\n");
        printf("\tcmp rax, 0\n");
        printf("\tje .Lend%d\n", count);
        gen(node->rhs);
        printf("\tjmp .Lbegin%d\n", count);
        printf(".Lend%d:\n", count);
        return;
    }

    if (node->kind == ND_FOR) {
        count++;
        if (node->init) {
            gen(node->init);
        }
        printf(".Lbegin%d:\n", count);
        if (node->cond) {
            gen(node->cond);
            printf("\tpop rax\n");
            printf("\tcmp rax, 0\n");
            printf("\tje .Lend%d\n", count);
        }
        gen(node->inc);
        if (node->body) {
            gen(node->body);
        }
        printf("\tjmp .Lbegin%d\n", count);
        printf(".Lend%d:\n", count);
        return;
    }

    if (node->kind == ND_FUNCDEF) {
        printf(".global %s\n", node->funcname);
        printf("%s:\n", node->funcname);

        printf("\tpush rbp\n");
        printf("\tmov rbp, rsp\n");

        int stack_size = locals_size();
        printf("\tsub rsp, %d\n", stack_size);

        gen(node->body);

        printf("\tmov rsp, rbp\n");
        printf("\tpop rbp\n");
        printf("\tret\n");

        return;
    }

    if (node->kind == ND_BLOCK) {
        for (int i = 0; i < node->stmt_count; i++) {
            gen(node->stmts[i]);
        }
        return;
    }

    if (node->kind == ND_CALL) {
        printf("\tcall %s\n", node->funcname);
        printf("\tpush rax\n"); 
        return;
    }

    switch (node->kind) {
    case ND_NUM:
        printf("\tpush %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("\tpop rax\n");
        printf("\tmov rax, [rax]\n");
        printf("\tpush rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);
        printf("\tpop rdi\n");
        printf("\tpop rax\n");
        printf("\tmov [rax], rdi\n");
        printf("\tpush rdi\n");
        return;
    case ND_ADDR:
        gen_lval(node->lhs);
        return;
    case ND_DEREF:
        gen(node->lhs);
        printf("\tpop rax\n");
        printf("\tmov rax, [rax]\n");
        printf("\tpush rax\n");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("\tpop rdi\n");
    printf("\tpop rax\n");

    switch (node->kind) {
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
        printf("\tcqo\n");
        printf("\tidiv rdi\n");
        break;
    case ND_LT:
        printf("\tcmp rax, rdi\n");
        printf("\tsetl al\n");
        printf("\tmovzb rax, al\n");
        break;
    case ND_GT:
        printf("\tcmp rax, rdi\n");
        printf("\tsetg al\n");
        printf("\tmovzb rax, al\n");
        break;
    case ND_LE:
        printf("\tcmp rax, rdi\n");
        printf("\tsetle al\n");
        printf("\tmovzb rax, al\n");
        break;
    case ND_GE:
        printf("\tcmp rax, rdi\n");
        printf("\tsetge al\n");
        printf("\tmovzb rax, al\n");
        break;
    case ND_EQ:
        printf("\tcmp rax, rdi\n");
        printf("\tsete al\n");
        printf("\tmovzb rax, al\n");
        break;
    case ND_NE:
        printf("\tcmp rax, rdi\n");
        printf("\tsetne al\n");
        printf("\tmovzb rax, al\n");
        break;
    }

    printf("\tpush rax\n");
}