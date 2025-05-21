.intel_syntax noprefix
.globl main
main:
	push 2
	push 3
ADD
	push 2
	push 4
ADD
ADD
	pop rax
	ret
