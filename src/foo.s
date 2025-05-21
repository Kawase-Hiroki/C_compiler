.intel_syntax noprefix
.global main
main:
	push 12
	push 34
	pop rdi
	pop rax
	add rax, rdi
	push rax
	push 5
	pop rdi
	pop rax
	sub rax, rdi
	push rax
	push 5
	pop rdi
	pop rax
	cmp rax, rdi
	setl al
	movzb rax, al
	push rax
	pop rax
	ret
