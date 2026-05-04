section .text

;========== FUNCTION "add" ==========
func_0:
	push rbp
	mov rbp, rsp
	sub rsp, 16		; Stack preparation

	movsd xmm0, [rbp + 16]
	movsd [rbp - 8], xmm0		; Take argument 1

	movsd xmm0, [rbp + 24]
	movsd [rbp - 16], xmm0		; Take argument 2

;========== RET ==========
	movsd xmm0, [rbp - 16]
	sub rsp, 8
	movsd [rsp], xmm0		; Save temporary value
	movsd xmm0, [rbp - 8]
	addsd xmm0, [rsp]
	add rsp, 8		; Operation complete

	jmp func_end_0

func_end_0:
	add rsp, 16
	pop rbp
	ret		; Stack free

;========== FUNCTION "main" ==========
func_3:
	push rbp
	mov rbp, rsp
	sub rsp, 0		; Stack preparation

;========== RET ==========
;===== CALL "add" =====
	sub rsp, 16
	movsd xmm0, [rel const_0]
	movsd [rsp + 0], xmm0		; Save func argument
	movsd xmm0, [rel const_1]
	movsd [rsp + 8], xmm0		; Save func argument
	call func_0

	add rsp, 16
	jmp func_end_3

func_end_3:
	add rsp, 0
	pop rbp
	ret		; Stack free

section .rodata

const_true:
	dq 1.0
const_false:
	dq 0.0
const_0:
	dq 10.000000000000000
const_1:
	dq 20.000000000000000
