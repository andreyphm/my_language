;*******************************************************;
;==================== PROGRAM START ====================;
;================== GitHub: andreyphm ==================;
;*******************************************************;

section .text

;==================== MY_STDLIB ====================;
;---------------------------------------------------------------------------------------------------------------------
; Terminates the process with exit code 0
;
; Arguments:    -
; Return value: -
; Destroy:      rax, rdi
;---------------------------------------------------------------------------------------------------------------------
__exit:
	mov rax, 60				; __x64_sys_exit
	xor rdi, rdi			; rdi = error_code
	syscall

;---------------------------------------------------------------------------------------------------------------------
; Prints double (6 digits in the fractional part) to stdout followed by a newline
;
; Arguments:    xmm0 = print value
; Return value: -
; Destroy:      rax, rbx, rcx, rdx, rdi, rsi, xmm0, xmm1
;---------------------------------------------------------------------------------------------------------------------
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
    xorpd xmm1, xmm1
    subsd xmm1, xmm0
    movsd xmm0, xmm1
	mov rax, 1				; sys_write
	mov rdi, 1				; stdout
	lea rsi, [rbp - 9]		; Buffer start
	mov rdx, 1				; Number of bytes to write
	syscall

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
	jmp .out_int_write

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
.out_int_write:
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

;---------------------------------------------------------------------------------------------------------------------
; Reads double from stdin
;
; Arguments:    -
; Return value: xmm0 = value read from stdin
; Destroy:      rax, rcx, rdx, rdi, rsi, xmm1, xmm2
;---------------------------------------------------------------------------------------------------------------------
__in:
    push rbp
    mov rbp, rsp
    push rbx                ; Used as sign flag
    sub rsp, 16

    xorpd xmm0, xmm0                ; xmm0 = result accumulator
    movsd xmm1, [rel __stdlib_01]   ; xmm1 = 0.1 (fractional digit weight)
    xor rbx, rbx                    ; rbx = sign (positive 0 negative 1)
    mov dl, 0
    mov [rbp - 10], dl              ; [rbp - 10] = mode (int 0 or frac 1)

.in_read:
    mov rax, 0                  ; sys_read
    mov rdi, 0                  ; stdin
    lea rsi, [rbp - 9]          ; Buffer start
    mov rdx, 1                  ; Number of bytes to read
    syscall
    movzx rax, [rbp - 9]        ; al = read character
    cmp al, 10                  ; '\n' check
    je .in_done
    cmp al, '-'
    jne .in_not_minus
    mov rbx, 1                  ; Set negative sign flag
    jmp .in_read
.in_not_minus:
    cmp al, '.'
    jne .in_digit
    mov dl, 1
    mov [rbp - 10], dl          ; Switch to fractional mode
    jmp .in_read

.in_digit:
    sub al, '0'                 ; Convert ASCII digit to value
    movzx rax, al
    cvtsi2sd xmm2, rax          ; xmm2 = digit as double
    movzx rax, [rbp - 10]
    test rax, rax
    jnz .in_frac
    mulsd xmm0, [rel __stdlib_10]       ; xmm0 *= 10
    addsd xmm0, xmm2                    ; xmm0 += digit
    jmp .in_read

.in_frac:
    mulsd xmm2, xmm1                    ; digit *= current weight (0.1, 0.01, ...)
    addsd xmm0, xmm2                    ; xmm0 += weighted digit
    mulsd xmm1, [rel __stdlib_01]       ; weight /= 10 (next digit is 10x smaller)
    jmp .in_read

.in_done:
    test rbx, rbx
    jz .in_positive
    xorpd xmm1, xmm1
    subsd xmm1, xmm0
    movsd xmm0, xmm1

.in_positive:
    add rsp, 16
    pop rbx
    pop rbp
    ret

main:
;==================== FUNCTION "main" ====================;
func_1:
	push rbp
	mov rbp, rsp
	sub rsp, 32					; Stack preparation

;==================== VAR_DECL_ID 0 "a" ====================;
;==================== IN ====================;
	call __in

	movsd [rbp - 8], xmm0		; variable_0 initialize

;==================== VAR_DECL_ID 1 "b" ====================;
;==================== IN ====================;
	call __in

	movsd [rbp - 16], xmm0		; variable_1 initialize

;==================== VAR_DECL_ID 2 "c" ====================;
;==================== IN ====================;
	call __in

	movsd [rbp - 24], xmm0		; variable_2 initialize

;================= CALL "equation_solver" =================;
	sub rsp, 24
	movsd xmm0, [rbp - 8]
	movsd [rsp + 0], xmm0		; Save func argument 1

	movsd xmm0, [rbp - 16]
	movsd [rsp + 8], xmm0		; Save func argument 2

	movsd xmm0, [rbp - 24]
	movsd [rsp + 16], xmm0		; Save func argument 3

	call func_6

	add rsp, 24
;==================== RET ====================;
	xorpd xmm0, xmm0
	jmp func_end_1

func_end_1:
	add rsp, 32
	pop rbp
	call __exit

;==================== FUNCTION "equation_solver" ====================;
func_6:
	push rbp
	mov rbp, rsp
	sub rsp, 32					; Stack preparation

	movsd xmm0, [rbp + 16]
	movsd [rbp - 8], xmm0		; Take argument 1

	movsd xmm0, [rbp + 24]
	movsd [rbp - 16], xmm0		; Take argument 2

	movsd xmm0, [rbp + 32]
	movsd [rbp - 24], xmm0		; Take argument 3

;==================== IF_1 ====================;
	movsd xmm0, [rel const_0]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rbp - 8]
	ucomisd xmm0, [rsp]
	je .cmp_true_1

	movsd xmm0, [rel const_false]
	jmp .cmp_end_1

.cmp_true_1:
	movsd xmm0, [rel const_true]

.cmp_end_1:
	add rsp, 8
	ucomisd xmm0, [rel const_false]	; Compare xmm0 with 0.0 (false)
	je .if_end_1

;================= CALL "linear_equation_solver" =================;
	sub rsp, 16
	movsd xmm0, [rbp - 16]
	movsd [rsp + 0], xmm0		; Save func argument 1

	movsd xmm0, [rbp - 24]
	movsd [rsp + 8], xmm0		; Save func argument 2

	call func_7

	add rsp, 16
	jmp .if_else_end_1

.if_end_1:
;==================== ELSE_1 ====================;
;================= CALL "quadratic_equation_solver" =================;
	sub rsp, 24
	movsd xmm0, [rbp - 8]
	movsd [rsp + 0], xmm0		; Save func argument 1

	movsd xmm0, [rbp - 16]
	movsd [rsp + 8], xmm0		; Save func argument 2

	movsd xmm0, [rbp - 24]
	movsd [rsp + 16], xmm0		; Save func argument 3

	call func_8

	add rsp, 24
.if_else_end_1:

;==================== RET ====================;
	xorpd xmm0, xmm0
	jmp func_end_6

func_end_6:
	add rsp, 32
	pop rbp
	ret

;==================== FUNCTION "linear_equation_solver" ====================;
func_7:
	push rbp
	mov rbp, rsp
	sub rsp, 16					; Stack preparation

	movsd xmm0, [rbp + 16]
	movsd [rbp - 8], xmm0		; Take argument 1

	movsd xmm0, [rbp + 24]
	movsd [rbp - 16], xmm0		; Take argument 2

;==================== IF_2 ====================;
	movsd xmm0, [rel const_1]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rbp - 8]
	ucomisd xmm0, [rsp]
	je .cmp_true_2

	movsd xmm0, [rel const_false]
	jmp .cmp_end_2

.cmp_true_2:
	movsd xmm0, [rel const_true]

.cmp_end_2:
	add rsp, 8
	ucomisd xmm0, [rel const_false]	; Compare xmm0 with 0.0 (false)
	je .if_end_2

;==================== IF_3 ====================;
	movsd xmm0, [rel const_2]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rbp - 16]
	ucomisd xmm0, [rsp]
	je .cmp_true_3

	movsd xmm0, [rel const_false]
	jmp .cmp_end_3

.cmp_true_3:
	movsd xmm0, [rel const_true]

.cmp_end_3:
	add rsp, 8
	ucomisd xmm0, [rel const_false]	; Compare xmm0 with 0.0 (false)
	je .if_end_3

;==================== OUT ====================;
	movsd xmm0, [rel const_3]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rel const_4]
	subsd xmm0, [rsp]
	add rsp, 8
	call __out

	jmp .if_else_end_3

.if_end_3:
;==================== ELSE_3 ====================;
;==================== OUT ====================;
	movsd xmm0, [rel const_5]
	call __out

.if_else_end_3:

	jmp .if_else_end_2

.if_end_2:
;==================== ELSE_2 ====================;
;==================== OUT ====================;
	movsd xmm0, [rel const_6]
	call __out

;==================== OUT ====================;
	movsd xmm0, [rbp - 8]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rbp - 16]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rel const_7]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rel const_8]
	subsd xmm0, [rsp]
	add rsp, 8
	mulsd xmm0, [rsp]
	add rsp, 8
	divsd xmm0, [rsp]
	add rsp, 8
	call __out

.if_else_end_2:

func_end_7:
	add rsp, 16
	pop rbp
	ret

;==================== FUNCTION "quadratic_equation_solver" ====================;
func_8:
	push rbp
	mov rbp, rsp
	sub rsp, 64					; Stack preparation

	movsd xmm0, [rbp + 16]
	movsd [rbp - 8], xmm0		; Take argument 1

	movsd xmm0, [rbp + 24]
	movsd [rbp - 16], xmm0		; Take argument 2

	movsd xmm0, [rbp + 32]
	movsd [rbp - 24], xmm0		; Take argument 3

;==================== VAR_DECL_ID 11 "D" ====================;
	movsd xmm0, [rbp - 24]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rbp - 8]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rel const_9]
	mulsd xmm0, [rsp]
	add rsp, 8
	mulsd xmm0, [rsp]
	add rsp, 8
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rbp - 16]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rbp - 16]
	mulsd xmm0, [rsp]
	add rsp, 8
	subsd xmm0, [rsp]
	add rsp, 8
	movsd [rbp - 32], xmm0		; variable_11 initialize

;==================== IF_4 ====================;
;================= CALL "is_close_to_zero" =================;
	sub rsp, 8
	movsd xmm0, [rbp - 32]
	movsd [rsp + 0], xmm0		; Save func argument 1

	call func_11

	add rsp, 8
	ucomisd xmm0, [rel const_false]	; Compare xmm0 with 0.0 (false)
	je .if_end_4

;==================== VAR_DECL_ID 12 "x1" ====================;
	movsd xmm0, [rbp - 8]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rel const_10]
	mulsd xmm0, [rsp]
	add rsp, 8
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rbp - 16]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rel const_11]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rel const_12]
	subsd xmm0, [rsp]
	add rsp, 8
	mulsd xmm0, [rsp]
	add rsp, 8
	divsd xmm0, [rsp]
	add rsp, 8
	movsd [rbp - 40], xmm0		; variable_12 initialize

;==================== OUT ====================;
	movsd xmm0, [rel const_13]
	call __out

;==================== OUT ====================;
	movsd xmm0, [rbp - 40]
	call __out

;==================== RET ====================;
	xorpd xmm0, xmm0
	jmp func_end_8

.if_end_4:
;==================== IF_5 ====================;
	movsd xmm0, [rel const_14]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rbp - 32]
	ucomisd xmm0, [rsp]
	ja .cmp_true_4

	movsd xmm0, [rel const_false]
	jmp .cmp_end_4

.cmp_true_4:
	movsd xmm0, [rel const_true]

.cmp_end_4:
	add rsp, 8
	ucomisd xmm0, [rel const_false]	; Compare xmm0 with 0.0 (false)
	je .if_end_5

;==================== VAR_DECL_ID 13 "x1" ====================;
	movsd xmm0, [rbp - 8]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rel const_15]
	mulsd xmm0, [rsp]
	add rsp, 8
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rbp - 32]
	sqrtsd xmm0, xmm0

	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rbp - 16]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rel const_16]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rel const_17]
	subsd xmm0, [rsp]
	add rsp, 8
	mulsd xmm0, [rsp]
	add rsp, 8
	addsd xmm0, [rsp]
	add rsp, 8
	divsd xmm0, [rsp]
	add rsp, 8
	movsd [rbp - 48], xmm0		; variable_13 initialize

;==================== VAR_DECL_ID 14 "x2" ====================;
	movsd xmm0, [rbp - 8]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rel const_18]
	mulsd xmm0, [rsp]
	add rsp, 8
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rbp - 32]
	sqrtsd xmm0, xmm0

	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rbp - 16]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rel const_19]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rel const_20]
	subsd xmm0, [rsp]
	add rsp, 8
	mulsd xmm0, [rsp]
	add rsp, 8
	subsd xmm0, [rsp]
	add rsp, 8
	divsd xmm0, [rsp]
	add rsp, 8
	movsd [rbp - 56], xmm0		; variable_14 initialize

;==================== OUT ====================;
	movsd xmm0, [rel const_21]
	call __out

;==================== OUT ====================;
	movsd xmm0, [rbp - 48]
	call __out

;==================== OUT ====================;
	movsd xmm0, [rbp - 56]
	call __out

;==================== RET ====================;
	xorpd xmm0, xmm0
	jmp func_end_8

	jmp .if_else_end_5

.if_end_5:
;==================== ELSE_5 ====================;
;==================== OUT ====================;
	movsd xmm0, [rel const_22]
	call __out

.if_else_end_5:

;==================== RET ====================;
	xorpd xmm0, xmm0
	jmp func_end_8

func_end_8:
	add rsp, 64
	pop rbp
	ret

;==================== FUNCTION "abs" ====================;
func_15:
	push rbp
	mov rbp, rsp
	sub rsp, 16					; Stack preparation

	movsd xmm0, [rbp + 16]
	movsd [rbp - 8], xmm0		; Take argument 1

;==================== IF_6 ====================;
	movsd xmm0, [rel const_23]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rbp - 8]
	ucomisd xmm0, [rsp]
	jb .cmp_true_5

	movsd xmm0, [rel const_false]
	jmp .cmp_end_5

.cmp_true_5:
	movsd xmm0, [rel const_true]

.cmp_end_5:
	add rsp, 8
	ucomisd xmm0, [rel const_false]	; Compare xmm0 with 0.0 (false)
	je .if_end_6

;==================== RET ====================;
	movsd xmm0, [rbp - 8]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	movsd xmm0, [rel const_24]
	subsd xmm0, [rsp]
	add rsp, 8
	jmp func_end_15

.if_end_6:
;==================== RET ====================;
	movsd xmm0, [rbp - 8]
	jmp func_end_15

func_end_15:
	add rsp, 16
	pop rbp
	ret

;==================== FUNCTION "is_close_to_zero" ====================;
func_11:
	push rbp
	mov rbp, rsp
	sub rsp, 16					; Stack preparation

	movsd xmm0, [rbp + 16]
	movsd [rbp - 8], xmm0		; Take argument 1

;==================== VAR_DECL_ID 17 "epsilon" ====================;
	movsd xmm0, [rel const_25]
	movsd [rbp - 16], xmm0		; variable_17 initialize

;==================== IF_7 ====================;
	movsd xmm0, [rbp - 16]
	sub rsp, 8
	movsd [rsp], xmm0			; Save temporary value

	sub rsp, 8					; Stack alignment before call
;================= CALL "abs" =================;
	sub rsp, 8
	movsd xmm0, [rbp - 8]
	movsd [rsp + 0], xmm0		; Save func argument 1

	call func_15

	add rsp, 8
	add rsp, 8					; Remove alignment
	ucomisd xmm0, [rsp]
	jb .cmp_true_6

	movsd xmm0, [rel const_false]
	jmp .cmp_end_6

.cmp_true_6:
	movsd xmm0, [rel const_true]

.cmp_end_6:
	add rsp, 8
	ucomisd xmm0, [rel const_false]	; Compare xmm0 with 0.0 (false)
	je .if_end_7

;==================== RET ====================;
	movsd xmm0, [rel const_26]
	jmp func_end_11

.if_end_7:
;==================== RET ====================;
	movsd xmm0, [rel const_27]
	jmp func_end_11

func_end_11:
	add rsp, 16
	pop rbp
	ret

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
__stdlib_10:
	dq 10.0
__stdlib_01:
	dq 0.1
const_0:
	dq 0.0000000000000000
const_1:
	dq 0.0000000000000000
const_2:
	dq 0.0000000000000000
const_3:
	dq 1.0000000000000000
const_4:
	dq 0.0000000000000000
const_5:
	dq 0.0000000000000000
const_6:
	dq 1.0000000000000000
const_7:
	dq 1.0000000000000000
const_8:
	dq 0.0000000000000000
const_9:
	dq 4.0000000000000000
const_10:
	dq 2.0000000000000000
const_11:
	dq 1.0000000000000000
const_12:
	dq 0.0000000000000000
const_13:
	dq 1.0000000000000000
const_14:
	dq 0.0000000000000000
const_15:
	dq 2.0000000000000000
const_16:
	dq 1.0000000000000000
const_17:
	dq 0.0000000000000000
const_18:
	dq 2.0000000000000000
const_19:
	dq 1.0000000000000000
const_20:
	dq 0.0000000000000000
const_21:
	dq 2.0000000000000000
const_22:
	dq 0.0000000000000000
const_23:
	dq 0.0000000000000000
const_24:
	dq 0.0000000000000000
const_25:
	dq 9.9999999999999995e-07
const_26:
	dq 1.0000000000000000
const_27:
	dq 0.0000000000000000
