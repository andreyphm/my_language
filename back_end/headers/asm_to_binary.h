#ifndef ASM_TO_BINARY_H
#define ASM_TO_BINARY_H

#include <stdio.h>
#include <stdint.h>  

enum operand_kind_t
{
    NOT_OPERAND,
    OPERAND_REG,
    OPERAND_XMM,
    OPERAND_MEM,
    OPERAND_IMM,
    OPERAND_LABEL
};

struct operand_t
{
    operand_kind_t kind;
    size_t reg_num;
    int64_t displacement;
    int64_t imm_value;
    char label_name[64];
};

struct instruction_t
{
    char mnemonic[16];
    operand_t operands[2];
    size_t operand_count;
    size_t encoded_size;
};

struct instruction_list_t
{
    instruction_t* instructions;
    size_t count;
    size_t capacity;
}

void asm_to_binary(FILE* const asm_file, FILE* const binary_file);

#endif // ASM_TO_BINARY_H
