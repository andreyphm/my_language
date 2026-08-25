#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "instructions_to_binary.h"

static bool fits_in_int8(int64_t value)
{
    return value >= -128 && value <= 127;
}

size_t get_instruction_size(const instruction_t* instruction)
{
    assert(instruction);

    const char* mnemonic = instruction->mnemonic;
    if (mnemonic[0] == '\0') return 0;

    const operand_t* first_op = &instruction->operands[0];
    const operand_t* second_op = &instruction->operands[1];

    if (!strcmp(mnemonic, "syscall")) return 2;
    if (!strcmp(mnemonic, "ret")) return 1;
    if (!strcmp(mnemonic, "push") || !strcmp(mnemonic, "pop")) return 1;
    if (!strcmp(mnemonic, "dq")) return 8;
    if (!strcmp(mnemonic, "xor") || !strcmp(mnemonic, "test")) return 3;

    if (!strcmp(mnemonic, "mov"))
    {
        if (first_op->kind == OPERAND_REG && second_op->kind == OPERAND_IMM)
            return (first_op->reg_size == 1) ? 2 : 7;
        if (first_op->kind == OPERAND_REG && second_op->kind == OPERAND_REG)
            return 3;
        if (first_op->kind == OPERAND_MEM && second_op->kind == OPERAND_REG)
            return (first_op->displacement == 0) ? 2 : 3;
    }

    if (!strcmp(mnemonic, "add"))
        return (first_op->reg_size == 1) ? 3 : (fits_in_int8(second_op->imm_value) ? 4 : 7);

    if (!strcmp(mnemonic, "sub"))
        return (first_op->reg_size == 1) ? 3 : (fits_in_int8(second_op->imm_value) ? 4 : 7);

    if (!strcmp(mnemonic, "cmp"))
        return (first_op->reg_size == 1) ? 3 : (fits_in_int8(second_op->imm_value) ? 4 : 7);

    if (!strcmp(mnemonic, "inc") || !strcmp(mnemonic, "dec")) return 3;
    if (!strcmp(mnemonic, "div")) return 3;
    if (!strcmp(mnemonic, "lea")) return 4;

    if (!strcmp(mnemonic, "movzx"))
        return (second_op->kind == OPERAND_REG) ? 4 : 5;

    if (!strcmp(mnemonic, "movsd"))
    {
        if (first_op->kind == OPERAND_MEM_REL || second_op->kind == OPERAND_MEM_REL)
            return 8;

        const operand_t* memory_op = first_op->kind == OPERAND_MEM ? first_op : second_op;
        if (memory_op->kind == OPERAND_MEM && !fits_in_int8(memory_op->displacement))
            return memory_op->reg_num == 4 ? 9 : 8;

        if (memory_op->kind == OPERAND_MEM && memory_op->reg_num == 4 && memory_op->displacement != 0)
            return 6;

        if (first_op->kind == OPERAND_XMM && second_op->kind == OPERAND_XMM)
            return 4;

        return 5;
    }

    if (!strcmp(mnemonic, "xorpd"))
        return (second_op->kind == OPERAND_MEM_REL) ? 8 : 4;

    if (!strcmp(mnemonic, "ucomisd"))
    {
        if (second_op->kind == OPERAND_XMM) return 4;
        if (second_op->kind == OPERAND_MEM_REL) return 8;
        return 5;
    }

    if (!strcmp(mnemonic, "cvttsd2si") || !strcmp(mnemonic, "cvtsi2sd")) return 5;

    if (!strcmp(mnemonic, "addsd") || !strcmp(mnemonic, "subsd") ||
        !strcmp(mnemonic, "mulsd") || !strcmp(mnemonic, "divsd") || !strcmp(mnemonic, "sqrtsd"))
    {
        if (second_op->kind == OPERAND_XMM)     return 4;
        if (second_op->kind == OPERAND_MEM_REL) return 8;
        return 5;
    }

    if (!strcmp(mnemonic, "jmp") || !strcmp(mnemonic, "call")) return 5;

    if (!strcmp(mnemonic, "je")  || !strcmp(mnemonic, "jz")  ||
        !strcmp(mnemonic, "jne") || !strcmp(mnemonic, "jnz") ||
        !strcmp(mnemonic, "jl")  || !strcmp(mnemonic, "jge") ||
        !strcmp(mnemonic, "jle") || !strcmp(mnemonic, "jg")  ||
        !strcmp(mnemonic, "ja")  || !strcmp(mnemonic, "jb")  ||
        !strcmp(mnemonic, "jae") || !strcmp(mnemonic, "jbe")) return 6;

    fprintf(stderr, "Unknown mnemonic '%s'\n", mnemonic);
    assert(0);
    return 0;
}

void calculate_sizes(instruction_list_t* list)
{
    assert(list);

    for (size_t i = 0; i < list->count; i++)
        list->instructions[i].encoded_size = get_instruction_size(&list->instructions[i]);
}

void compute_labels_addresses(instruction_list_t* list, label_list_t* labels,
                              size_t rodata_instruction_index,
                              uint64_t text_start, uint64_t rodata_start)
{
    assert(list);
    assert(labels);
    assert(rodata_instruction_index <= list->count);

    for (size_t i = 0; i < labels->count; i++)
    {
        size_t index = labels->labels[i].instruction_index;
        assert(index <= list->count);

        bool is_rodata = index >= rodata_instruction_index;
        uint64_t address = is_rodata ? rodata_start : text_start;
        size_t first_instruction = is_rodata ? rodata_instruction_index : 0;

        for (size_t j = first_instruction; j < index; j++)
            address += list->instructions[j].encoded_size;

        labels->labels[i].address = address;
    }
}
