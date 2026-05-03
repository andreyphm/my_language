section .text

;========== FUNCTION "main" ==========
func_1:
	push rbp
	mov rbp, rsp
	sub rsp, 16		; Stack preparation

;========== VAR_DECL_ID 0 "x"==========
	movsd xmm0, [rel const_0]
	movsd [rbp - 8], xmm0		; variable_0 init

	movsd xmm0, [rbp - 8]
	movsd xmm1, xmm0		; Save right value in xmm1
	movsd xmm0, [rel const_1]
	ucomisd xmm0, xmm1
	jb .cmp_true_0

	movsd xmm0, [rel const_false]
	jmp cmp_end_0

cmp_true_0:
	movsd xmm0, [rel const_true]

cmp_end_0:		; Operation complete

	movsd [rbp - 8], xmm0		; Operation complete

;========== RET ==========
	movsd xmm0, [rbp - 8]
	jmp func_end_1

func_end_1:
	add rsp, 16
	pop rbp
	ret		; Stack free

section .rodata

const_true:
	dq 1.0
const_false:
	dq 0.0
const_0:
	dq 0
const_1:
	dq 5
