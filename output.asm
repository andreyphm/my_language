section .text

;========== FUNCTION "main" ==========
func_0:
	push rbp
	mov rbp, rsp
	sub rsp, 16		; Stack preparation

;========== VAR_DECL_ID 0 "x" ==========
	movsd xmm0, [rel const_0]
	movsd [rbp - 8], xmm0		; variable_0 init

	call func_2

func_end_0:
	add rsp, 16
	pop rbp
	ret		; Stack free

;========== FUNCTION "koshka" ==========
func_2:
	push rbp
	mov rbp, rsp
	sub rsp, 16		; Stack preparation

;========== VAR_DECL_ID 1 "x" ==========
	movsd xmm0, [rel const_1]
	movsd [rbp - 8], xmm0		; variable_1 init

;========== RET ==========
	xorpd xmm0, xmm0
	jmp func_end_2

func_end_2:
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
	dq 3.0000000000000000
