%ifndef MY_STDLIB_ASM
%define MY_STDLIB_ASM

extern printf
extern scanf

section .rodata

__out_fmt: db "%g", 10, 0
__in_fmt: db "%lf", 0

%endif
