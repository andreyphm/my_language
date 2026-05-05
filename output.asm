section .text

;========== FUNCTION "fact" ==========
func_0:
	push rbp
	mov rbp, rsp
	sub rsp, 16		; Stack preparation

	movsd xmm0, [rbp + 16]
	movsd [rbp - 8], xmm0		; Take argument 1

;========== IF_1 ==========
	movsd xmm0, [rel const_0]
	sub rsp, 8
	movsd [rsp], xmm0		; Save temporary value

	movsd xmm0, [rbp - 8]
	ucomisd xmm0, [rsp]
	jbe .cmp_true_1

	movsd xmm0, [rel const_false]
	jmp .cmp_end_1

.cmp_true_1:
	movsd xmm0, [rel const_true]

.cmp_end_1:
	add rsp, 8		; Operation complete

	ucomisd xmm0, [rel const_false]
	je .if_end_1

;========== RET ==========
	movsd xmm0, [rel const_1]
	jmp func_end_0

	jmp .if_else_end_1

.if_end_1:
;========== ELSE_1 ==========
;========== RET ==========
;===== CALL "fact" =====
	sub rsp, 8

	movsd xmm0, [rel const_2]
	sub rsp, 8
	movsd [rsp], xmm0		; Save temporary value

	movsd xmm0, [rbp - 8]
	subsd xmm0, [rsp]
	add rsp, 8		; Operation complete

	movsd [rsp + 0], xmm0		; Save func argument 1

	call func_0

	add rsp, 8
	sub rsp, 8
	movsd [rsp], xmm0		; Save temporary value

	movsd xmm0, [rbp - 8]
	mulsd xmm0, [rsp]
	add rsp, 8		; Operation complete

	jmp func_end_0

.if_else_end_1:

func_end_0:
	add rsp, 16
	pop rbp
	ret		; Stack free

;========== FUNCTION "main" ==========
func_2:
	push rbp
	mov rbp, rsp
	sub rsp, 0		; Stack preparation

;========== RET ==========
;===== CALL "fact" =====
	sub rsp, 8

	movsd xmm0, [rel const_3]
	movsd [rsp + 0], xmm0		; Save func argument 1

	call func_0

	add rsp, 8
	sub rsp, 8
	movsd [rsp], xmm0		; Save temporary value

;===== CALL "fact" =====
	sub rsp, 8

	movsd xmm0, [rel const_4]
	movsd [rsp + 0], xmm0		; Save func argument 1

	call func_0

	add rsp, 8
	addsd xmm0, [rsp]
	add rsp, 8		; Operation complete

	jmp func_end_2

func_end_2:
	add rsp, 0
	pop rbp
	ret		; Stack free

section .rodata

const_true:
	dq 1.0
const_false:
	dq 0.0
const_0:
	dq 1.0000000000000000
const_1:
	dq 1.0000000000000000
const_2:
	dq 1.0000000000000000
const_3:
	dq 3.0000000000000000
const_4:
	dq 5.0000000000000000
