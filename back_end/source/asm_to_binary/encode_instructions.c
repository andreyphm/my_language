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
    assert(buffer_pos);
    assert(*buffer_pos);

    const char* mnemonic = instruction->mnemonic;
    const operand_t* first_op = &instruction->operands[0];
    const operand_t* second_op = &instruction->operands[1];

    if (!strcmp(mnemonic, "syscall")) // Bytes: 0x0F | 0x05
    {
        emit_1_byte(buffer_pos, 0x0F);
        emit_1_byte(buffer_pos, 0x05);
        return;
    }

    if (!strcmp(mnemonic, "ret")) // Byte: 0xC3
    {
        emit_1_byte(buffer_pos, 0xC3);
        return;
    }

    if (!strcmp(mnemonic, "push")) // Byte: 0x50 + reg_num
    {
        emit_1_byte(buffer_pos, (uint8_t) (0x50 + first_op->reg_num));
        return;
    }

    if (!strcmp(mnemonic, "pop")) // Byte: 0x58 + reg_num
    {
        emit_1_byte(buffer_pos, (uint8_t) (0x58 + first_op->reg_num));
        return;
    }

    if (!strcmp(mnemonic, "dq")) // Bytes: double_value(8)
    {
        uint64_t bytes = 0;
        memcpy(&bytes, &first_op->double_value, 8);
        emit_8_bytes(buffer_pos, bytes);
        return;
    }

    if (!strcmp(mnemonic, "xor")) // Bytes: REX.W(0x48) | 0x33 | ModRM
    {
        uint8_t op_code  = 0x33;
        uint8_t mod_rm   = (uint8_t) ((3 << 6) | (first_op->reg_num << 3) | second_op->reg_num);

        emit_1_byte(buffer_pos, 0x48);
        emit_1_byte(buffer_pos, op_code);
        emit_1_byte(buffer_pos, mod_rm);
        return;
    }

    if (!strcmp(mnemonic, "test")) // Bytes: REX.W(0x48) | 0x85 | ModRM
    {
        uint8_t op_code  = 0x85;
        uint8_t mod_rm   = (uint8_t) ((3 << 6) | (first_op->reg_num << 3) | second_op->reg_num);

        emit_1_byte(buffer_pos, 0x48);
        emit_1_byte(buffer_pos, op_code);
        emit_1_byte(buffer_pos, mod_rm);
        return;
    }

    if (!strcmp(mnemonic, "mov"))
    {
        if (first_op->kind == OPERAND_REG && second_op->kind == OPERAND_IMM)
        {
            if (first_op->reg_size == 1) // Bytes: 0xB0 + reg_num | imm8
            {
                emit_1_byte(buffer_pos, (uint8_t) (0xB0 + first_op->reg_num));
                emit_1_byte(buffer_pos, (uint8_t) second_op->imm_value);
                return;
            }

            else // Bytes: REX.W(0x48) | 0xC7 | ModRM | imm32(4)
            {
                uint8_t mod_rm = (uint8_t) ((3 << 6) | first_op->reg_num);

                emit_1_byte(buffer_pos, 0x48);
                emit_1_byte(buffer_pos, 0xC7);
                emit_1_byte(buffer_pos, mod_rm);
                emit_4_bytes(buffer_pos, (uint32_t) second_op->imm_value);
                return;
            }
        }

        if (first_op->kind == OPERAND_REG && second_op->kind == OPERAND_REG) // Bytes: REX.W(0x48) | 0x89 | ModRM
        {
            uint8_t mod_rm = (uint8_t)((3 << 6) | (second_op->reg_num << 3) | first_op->reg_num);

            emit_1_byte(buffer_pos, 0x48);
            emit_1_byte(buffer_pos, 0x89);
            emit_1_byte(buffer_pos, mod_rm);
            return;
        }

        if (first_op->kind == OPERAND_MEM && second_op->kind == OPERAND_REG)
        {
            if (first_op->displacement == 0) // Bytes: 0x88 | ModRM
            {
                uint8_t mod_rm = (uint8_t)((second_op->reg_num << 3) | first_op->reg_num);

                emit_1_byte(buffer_pos, 0x88);
                emit_1_byte(buffer_pos, mod_rm);
            }
            else // Bytes: 0x88 | ModRM | disp8 
            {
                uint8_t mod_rm = (uint8_t)((1 << 6) | (second_op->reg_num << 3) | first_op->reg_num);

                emit_1_byte(buffer_pos, 0x88);
                emit_1_byte(buffer_pos, mod_rm);
                emit_1_byte(buffer_pos, (uint8_t) first_op->displacement);
            }
            return;
        }
    }

    fprintf(stderr, "Unknown mnemonic '%s'\n", mnemonic);
    assert(0);
}

void emit_1_byte(uint8_t** buffer_pos, uint8_t byte)
{
    assert(buffer_pos);
    assert(*buffer_pos);

    **buffer_pos = byte;
    (*buffer_pos)++;
}

void emit_4_bytes(uint8_t** buffer_pos, uint32_t value)
{
    assert(buffer_pos);
    assert(*buffer_pos);

    memcpy(*buffer_pos, &value, 4);
    *buffer_pos += 4;
}

void emit_8_bytes(uint8_t** buffer_pos, uint64_t value)
{
    assert(buffer_pos);
    assert(*buffer_pos);

    memcpy(*buffer_pos, &value, 8);
    *buffer_pos += 8;
}
