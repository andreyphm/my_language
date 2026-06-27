;*******************************************************;
;==================== PROGRAM START ====================;
;================== GitHub: andreyphm ==================;
;*******************************************************;

section .text

;==================== MY_STDLIB ====================;
__exit:
	mov rax, 60				; __x64_sys_exit
	xor rdi, rdi			; rdi = error_code
	syscall

__out:
	push rbp
	mov rbp, rsp
	push rbx				; Used as divisor = 10
	sub rsp, 32

	xorpd xmm1, xmm1
	ucomisd xmm0, xmm1
	jae .out_pos			; if (value >= 0) jump, else write '-' and turn xmm0 to positive

	mov dl, '-'
	mov [rbp - 9], dl
	mov rax, 1				; sys_write
	mov rdi, 1				; stdout
	lea rsi, [rbp - 9]		; Buffer start
	mov rdx, 1				; Number of bytes to write
	syscall
	xorpd xmm0, [rel __stdlib_neg0]		; xmm0 to positive

.out_pos:
	cvttsd2si rax, xmm0			; rax = integer part of xmm0 (truncated)
	cvtsi2sd xmm1, rax			; xmm1 = integer part of xmm0 as double
	subsd xmm0, xmm1			; Now xmm0 = fractional part
	lea rdi, [rbp - 10]			; Start of buffer
	xor rcx, rcx				; rcx = digits counter

	test rax, rax
	jnz .out_int_loop			; Special case: rax = 0
	mov dl, '0'
	mov [rbp - 10], dl
	mov rcx, 1
	jmp .out_int_done

.out_int_loop:
	test rax, rax
	jz .out_int_done
	xor rdx, rdx				; rdx = 0 before div (dividend = rdx:rax)
	mov rbx, 10
	div rbx						; rax /= 10, rdx = rax % 10
	add dl, '0'
	mov [rdi], dl				; Save digit in buffer
	dec rdi
	inc rcx
	jmp .out_int_loop
.out_int_done:
	inc rdi					    ; rdi now points to most significant digit
	mov rax, 1				    ; sys_write
	mov rsi, rdi			    ; Buffer start
	mov rdx, rcx			    ; Number of bytes to write
	mov rdi, 1				    ; stdout
	syscall

	mov dl, '.'
	mov [rbp - 9], dl
	mov rax, 1				    ; sys_write
	mov rdi, 1				    ; stdout
	lea rsi, [rbp - 9]		    ; Buffer start
	mov rdx, 1				    ; Number of bytes to write
	syscall

	mulsd xmm0, [rel __stdlib_1m]		; xmm0 = fractional * 1.000.000
	cvttsd2si rax, xmm0					; rax = 6-digit int
	lea rdi, [rbp - 25]
	mov rcx, 6
.out_frac_loop:
	xor rdx, rdx			    ; rdx = 0 before div (dividend = rdx:rax)
	mov rbx, 10
	div rbx					    ; rax /= 10, rdx = rax % 10
	add dl, '0'
	mov [rdi], dl
	dec rdi
	dec rcx
	jnz .out_frac_loop
	inc rdi
	mov rax, 1				    ; sys_write
	mov rsi, rdi			    ; Buffer start
	mov rdx, 6				    ; Number of bytes to write
	mov rdi, 1				    ; stdout
	syscall

	mov dl, 10				    ; '\n' ASCII
	mov [rbp - 9], dl
	mov rax, 1				    ; sys_write
	mov rdi, 1				    ; stdout
	lea rsi, [rbp - 9]		    ; Buffer start
	mov rdx, 1				    ; Number of bytes to write
	syscall

	add rsp, 32
	pop rbx
	pop rbp
	ret
    
;==================== FUNCTION "fact" ====================;
func_1:
	push rbp
	mov rbp, rsp
	sub rsp, 16					; Stack preparation

	movsd xmm0, [rbp + 16]
	movsd [rbp - 8], xmm0		; Take argument 1

;==================== IF_1 ====================;
	movsd xmm0, [rel const_0]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rbp - 8]
	ucomisd xmm0, [rsp]
	jbe .cmp_true_1

	movsd xmm0, [rel const_false]
	jmp .cmp_end_1

.cmp_true_1:
	movsd xmm0, [rel const_true]

.cmp_end_1:
	add rsp, 8
	ucomisd xmm0, [rel const_false]	; Compare xmm0 with 0.0 (false)
	je .if_end_1

;==================== RET ====================;
	movsd xmm0, [rel const_1]
	jmp func_end_1

	jmp .if_else_end_1

.if_end_1:
;==================== ELSE_1 ====================;
;==================== RET ====================;
;================= CALL "fact" =================;
	sub rsp, 8
	movsd xmm0, [rel const_2]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rbp - 8]
	subsd xmm0, [rsp]
	add rsp, 8
	movsd [rsp + 0], xmm0		; Save func argument 1

	call func_1

	add rsp, 8
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rbp - 8]
	mulsd xmm0, [rsp]
	add rsp, 8
	jmp func_end_1

.if_else_end_1:

func_end_1:
	add rsp, 16
	pop rbp
	ret

main:
;==================== FUNCTION "main" ====================;
func_3:
	push rbp
	mov rbp, rsp
	sub rsp, 16					; Stack preparation

;==================== VAR_DECL_ID 1 "x" ====================;
	movsd xmm0, [rel const_3]
	movsd [rbp - 8], xmm0		; variable_1 initialize

;==================== IN ====================;
	call __in

	movsd [rbp - 8], xmm0
;==================== OUT ====================;
;================= CALL "fact" =================;
	sub rsp, 8
	movsd xmm0, [rbp - 8]
	movsd [rsp + 0], xmm0		; Save func argument 1

	call func_1

	add rsp, 8
	call __out

;==================== RET ====================;
	xorpd xmm0, xmm0
	jmp func_end_3

func_end_3:
	add rsp, 16
	pop rbp
	call __exit

;================= PROGRAM END =================;

section .rodata

const_true:
	dq 1.0
const_false:
	dq 0.0
__stdlib_neg0:
	dq -0.0
__stdlib_1m:
	dq 1000000.0
const_0:
	dq 1.0000000000000000
const_1:
	dq 1.0000000000000000
const_2:
	dq 1.0000000000000000
const_3:
	dq 0.0000000000000000
