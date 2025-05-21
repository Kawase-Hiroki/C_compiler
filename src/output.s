.globl main
main:
movq $2, %rax
push %rax
movq $3, %rax
push %rax
movq $4, %rax
movq %rax, %rbx
pop %rax
addq %rbx, %rax
movq %rax, %rbx
pop %rax
addq %rbx, %rax
movq %rax, %rdi
call exit
