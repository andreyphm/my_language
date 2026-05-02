section .text

;========== FUNCTION "main" ==========
func_1:
	push rbp
	mov rbp, rsp
	sub rsp, 16
	movsd xmm0, [rel const_0]
	movsd [rbp - 8], xmm0
	movsd xmm0, [rbp - 8]
	jmp func_end_1

func_end_1:
	add rsp, 16
	ret

section .rodata

const_0:
	dq 4.5
