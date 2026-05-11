%ifndef MY_STDLIB_ASM
%define MY_STDLIB_ASM

extern printf

section .rodata

__out_fmt: db "%g", 10, 0

%endif
