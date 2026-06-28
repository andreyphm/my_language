#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "asm_to_binary.h"

size_t get_instruction_size(const instruction_t* instruction)
{
    assert(instruction);

    const char* mnemonic = instruction->mnemonic;
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

void compute_labels_addresses(instruction_list_t* list, label_list_t* labels, uint64_t code_start)
{
    assert(list);
    assert(labels);

    for (size_t i = 0; i < labels->count; i++)
    {
        uint64_t address = code_start;
        size_t index = labels->labels[i].instruction_index;

        for (size_t j = 0; j < index; j++)
            address += list->instructions[j].encoded_size;

        labels->labels[i].address = address;
    }
}
