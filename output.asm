section .text

;========== FUNCTION "main" ==========
func_1:
	push rbp
	mov rbp, rsp
	sub rsp, 16		; Stack preparation

;========== VAR_DECL_ID 0 "x" ==========
	movsd xmm0, [rel const_0]
	movsd [rbp - 8], xmm0		; variable_0 init

;========== VAR_DECL_ID 1 "y" ==========
	movsd xmm0, [rel const_1]
	movsd [rbp - 16], xmm0		; variable_1 init

;========== WHILE_1 ==========
	movsd xmm0, [rel const_2]
	movsd [rbp - 24], xmm0		; Save value in stack
	movsd xmm0, [rbp - 8]
	ucomisd xmm0, [rbp - 24]
	jb .cmp_true_1

	movsd xmm0, [rel const_false]
	jmp .cmp_end_1

.cmp_true_1:
	movsd xmm0, [rel const_true]

.cmp_end_1:		; Operation complete

	ucomisd xmm0, [rel const_false]
	je .while_end_1

.while_loop_1:
	movsd xmm0, [rel const_3]
	movsd [rbp - 24], xmm0		; Save value in stack
	movsd xmm0, [rbp - 8]
	addsd xmm0, [rbp - 24]		; Operation complete

	movsd [rbp - 8], xmm0		; Operation complete

	movsd xmm0, [rel const_4]
	movsd [rbp - 16], xmm0		; Operation complete

;========== WHILE_2 ==========
	movsd xmm0, [rel const_5]
	movsd [rbp - 24], xmm0		; Save value in stack
	movsd xmm0, [rbp - 16]
	ucomisd xmm0, [rbp - 24]
	jb .cmp_true_2

	movsd xmm0, [rel const_false]
	jmp .cmp_end_2

.cmp_true_2:
	movsd xmm0, [rel const_true]

.cmp_end_2:		; Operation complete

	ucomisd xmm0, [rel const_false]
	je .while_end_2

.while_loop_2:
	movsd xmm0, [rel const_6]
	movsd [rbp - 24], xmm0		; Save value in stack
	movsd xmm0, [rbp - 16]
	addsd xmm0, [rbp - 24]		; Operation complete

	movsd [rbp - 16], xmm0		; Operation complete

;========== IF_1 ==========
	movsd xmm0, [rel const_7]
	movsd [rbp - 24], xmm0		; Save value in stack
	movsd xmm0, [rbp - 16]
	ucomisd xmm0, [rbp - 24]
	je .cmp_true_3

	movsd xmm0, [rel const_false]
	jmp .cmp_end_3

.cmp_true_3:
	movsd xmm0, [rel const_true]

.cmp_end_3:		; Operation complete

	ucomisd xmm0, [rel const_false]
	je .if_end_1

;========== BREAK ==========
jmp .while_end_2

.if_end_1:
	movsd xmm0, [rel const_8]
	movsd [rbp - 24], xmm0		; Save value in stack
	movsd xmm0, [rbp - 16]
	ucomisd xmm0, [rbp - 24]
	jb .cmp_true_4

	movsd xmm0, [rel const_false]
	jmp .cmp_end_4

.cmp_true_4:
	movsd xmm0, [rel const_true]

.cmp_end_4:		; Operation complete

	ucomisd xmm0, [rel const_false]
	jne .while_loop_2

.while_end_2:

	movsd xmm0, [rel const_9]
	movsd [rbp - 24], xmm0		; Save value in stack
	movsd xmm0, [rbp - 8]
	ucomisd xmm0, [rbp - 24]
	jb .cmp_true_5

	movsd xmm0, [rel const_false]
	jmp .cmp_end_5

.cmp_true_5:
	movsd xmm0, [rel const_true]

.cmp_end_5:		; Operation complete

	ucomisd xmm0, [rel const_false]
	jne .while_loop_1

.while_end_1:

;========== RET ==========
	movsd xmm0, [rbp - 16]
	movsd [rbp - 24], xmm0		; Save value in stack
	movsd xmm0, [rel const_10]
	movsd [rbp - 32], xmm0		; Save value in stack
	movsd xmm0, [rbp - 8]
	mulsd xmm0, [rbp - 32]		; Operation complete

	addsd xmm0, [rbp - 24]		; Operation complete

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
	dq 0.0000000000000000
const_1:
	dq 0.0000000000000000
const_2:
	dq 3.0000000000000000
const_3:
	dq 1.0000000000000000
const_4:
	dq 0.0000000000000000
const_5:
	dq 5.0000000000000000
const_6:
	dq 1.0000000000000000
const_7:
	dq 2.0000000000000000
const_8:
	dq 5.0000000000000000
const_9:
	dq 3.0000000000000000
const_10:
	dq 10.000000000000000
