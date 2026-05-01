section .text

;========== FUNCTION "main" ==========
func_1:
	push rbp
	mov rbp, rsp
	sub rsp, 0
	movsd xmm0, [rel const_0]

section .rodata

const_0:
	dq 0
