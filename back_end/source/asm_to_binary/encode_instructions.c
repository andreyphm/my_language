#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#include "asm_to_binary.h"

size_t encode_all(const instruction_list_t* list, const label_list_t* labels, uint64_t code_start, uint8_t* buffer)
{
    assert(list);
    assert(labels);
    assert(buffer);

    uint8_t* buffer_pos = buffer;
    uint64_t current_address = code_start;

    for (size_t i = 0; i < list->count; i++)
    {
        encode_instruction(&list->instructions[i], labels, current_address, &buffer_pos);
        current_address += list->instructions[i].encoded_size;
    }

    return (size_t) (buffer_pos - buffer);
}

void encode_instruction(const instruction_t* instruction, const label_list_t* labels,
                              uint64_t instruction_address, uint8_t** buffer_pos)
{
    assert(instruction);
    assert(labels);

    const char* mnemonic = instruction->mnemonic;
    const operand_t* first_op = &instruction->operands[0];
    const operand_t* second_op = &instruction->operands[1];

    if (!strcmp(mnemonic, "syscall"))
    {
        emit_1_byte(buffer_pos, 0x0F);
        emit_1_byte(buffer_pos, 0x05);
        return;
    }

    if (!strcmp(mnemonic, "ret"))
    {
        emit_1_byte(buffer_pos, 0xC3);
        return;
    }

    fprintf(stderr, "Unknown mnemonic '%s'\n", mnemonic);
    assert(0);
}

void emit_1_byte(uint8_t** buffer_pos, uint8_t byte)
{
    **buffer_pos = byte;
    (*buffer_pos)++;
}

void emit_4_bytes(uint8_t** buffer_pos, int32_t value)
{
    memcpy(*buffer_pos, &value, 4);
    *buffer_pos += 4;
}

void emit_8_bytes(uint8_t** buffer_pos, uint64_t value)
{
    memcpy(*buffer_pos, &value, 8);
    *buffer_pos += 8;
}
