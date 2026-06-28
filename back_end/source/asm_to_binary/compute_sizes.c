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
    if (!strcmp(mnemonic, "ret"))     return 1;

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
