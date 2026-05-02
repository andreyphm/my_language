section .text

;========== FUNCTION "main" ==========
func_1:
	push rbp
	mov rbp, rsp
	sub rsp, 16		; Stack preparation

;========== VAR_DECL_ID 0 ==========
	movsd xmm0, [rel const_0]
	movsd [rbp - 8], xmm0		; variable_0 ("x") init

;========== RET ==========
	movsd xmm0, [rel const_1]
	movsd xmm1, xmm0		; Save right value in xmm1

	movsd xmm0, [rbp - 8]
	divsd xmm0, xmm1		; Operation end

	jmp func_end_1

func_end_1:
	add rsp, 16
	pop rbp
	ret		; Stack free

section .rodata

const_0:
	dq 4.5
const_1:
	dq 4
