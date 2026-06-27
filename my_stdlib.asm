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
    xor rcx, rcx                    ; rcx = mode (int 0 or frac 1)

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
    mov rcx, 1                  ; Switch to fractional mode
    jmp .in_read

.in_digit:
    sub al, '0'                 ; Convert ASCII digit to value
    movzx rax, al
    cvtsi2sd xmm2, rax          ; xmm2 = digit as double
    test rcx, rcx
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
    xorpd xmm0, [rel __stdlib_neg0]     ; xmm0 to negative

.in_positive:
    add rsp, 16
    pop rbx
    pop rbp
    ret

