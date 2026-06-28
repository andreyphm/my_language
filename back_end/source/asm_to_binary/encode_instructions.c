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

void encode_instruction(const instruction_t* instruction,
                        const label_list_t* labels, uint64_t instruction_address, uint8_t** buffer_pos)
{
    assert(instruction);
    assert(labels);
    assert(buffer_pos);
    assert(*buffer_pos);

    const char* mnemonic = instruction->mnemonic;
    const operand_t* first_op = &instruction->operands[0];
    const operand_t* second_op = &instruction->operands[1];

    if (!strcmp(mnemonic, "syscall"))   { encode_syscall(buffer_pos);                    return; }
    if (!strcmp(mnemonic, "ret"))       { encode_ret(buffer_pos);                        return; }
    if (!strcmp(mnemonic, "push"))      { encode_push(first_op, buffer_pos);             return; }
    if (!strcmp(mnemonic, "pop"))       { encode_pop(first_op, buffer_pos);              return; }
    if (!strcmp(mnemonic, "dq"))        { encode_dq(first_op, buffer_pos);               return; }
    if (!strcmp(mnemonic, "xor"))       { encode_xor(first_op, second_op, buffer_pos);   return; }
    if (!strcmp(mnemonic, "test"))      { encode_test(first_op, second_op, buffer_pos);  return; }
    if (!strcmp(mnemonic, "mov"))       { encode_mov(first_op, second_op, buffer_pos);   return; }
    if (!strcmp(mnemonic, "add"))       { encode_add(first_op, second_op, buffer_pos);   return; }
    if (!strcmp(mnemonic, "sub"))       { encode_sub(first_op, second_op, buffer_pos);   return; }
    if (!strcmp(mnemonic, "cmp"))       { encode_cmp(first_op, second_op, buffer_pos);   return; }
    if (!strcmp(mnemonic, "inc"))       { encode_inc(first_op, buffer_pos);              return; }
    if (!strcmp(mnemonic, "dec"))       { encode_dec(first_op, buffer_pos);              return; }
    if (!strcmp(mnemonic, "div"))       { encode_div(first_op, buffer_pos);              return; }
    if (!strcmp(mnemonic, "lea"))       { encode_lea(first_op, second_op, buffer_pos);   return; }
    if (!strcmp(mnemonic, "movzx"))     { encode_movzx(first_op, second_op, buffer_pos); return; }
    if (!strcmp(mnemonic, "movsd"))     { encode_movsd(first_op, second_op, labels,
                                                       instruction_address, buffer_pos); return; }

    fprintf(stderr, "Unknown mnemonic '%s'\n", mnemonic);
    assert(0);
}

void encode_syscall(uint8_t** buffer_pos)
{
    // Bytes: 0x0F | 0x05
    emit_1_byte(buffer_pos, 0x0F);
    emit_1_byte(buffer_pos, 0x05);
}

void encode_ret(uint8_t** buffer_pos)
{
    // Byte: 0xC3
    emit_1_byte(buffer_pos, 0xC3);
}

void encode_push(const operand_t* op, uint8_t** buffer_pos)
{
    // Byte: 0x50 + reg_num
    emit_1_byte(buffer_pos, (uint8_t) (0x50 + op->reg_num));
}

void encode_pop(const operand_t* op, uint8_t** buffer_pos)
{
    // Byte: 0x58 + reg_num
    emit_1_byte(buffer_pos, (uint8_t) (0x58 + op->reg_num));
}

void encode_dq(const operand_t* op, uint8_t** buffer_pos)
{
    // Bytes: double_value(8)
    uint64_t bytes = 0;
    memcpy(&bytes, &op->double_value, sizeof(uint64_t));
    emit_8_bytes(buffer_pos, bytes);
}

void encode_xor(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos)
{
    // Bytes: REX.W(0x48) | 0x33 | ModRM
    uint8_t mod_rm = (uint8_t)((3 << 6) | (op0->reg_num << 3) | op1->reg_num);
    emit_1_byte(buffer_pos, 0x48);
    emit_1_byte(buffer_pos, 0x33);
    emit_1_byte(buffer_pos, mod_rm);
}

void encode_test(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos)
{
    // Bytes: REX.W(0x48) | 0x85 | ModRM
    uint8_t mod_rm = (uint8_t)((3 << 6) | (op0->reg_num << 3) | op1->reg_num);
    emit_1_byte(buffer_pos, 0x48);
    emit_1_byte(buffer_pos, 0x85);
    emit_1_byte(buffer_pos, mod_rm);
}

void encode_mov(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos)
{
    if (op0->kind == OPERAND_REG && op1->kind == OPERAND_IMM)
    {
        if (op0->reg_size == 1) // Bytes: 0xB0 + reg_num | imm8
        {
            emit_1_byte(buffer_pos, (uint8_t) (0xB0 + op0->reg_num));
            emit_1_byte(buffer_pos, (uint8_t) op1->imm_value);
        }
        else // Bytes: REX.W(0x48) | 0xC7 | ModRM | imm32(4)
        {
            uint8_t mod_rm = (uint8_t)((3 << 6) | op0->reg_num);
            emit_1_byte(buffer_pos, 0x48);
            emit_1_byte(buffer_pos, 0xC7);
            emit_1_byte(buffer_pos, mod_rm);
            emit_4_bytes(buffer_pos, (uint32_t) op1->imm_value);
        }
        return;
    }

    if (op0->kind == OPERAND_REG && op1->kind == OPERAND_REG) // Bytes: REX.W(0x48) | 0x89 | ModRM
    {
        uint8_t mod_rm = (uint8_t)((3 << 6) | (op1->reg_num << 3) | op0->reg_num);
        emit_1_byte(buffer_pos, 0x48);
        emit_1_byte(buffer_pos, 0x89);
        emit_1_byte(buffer_pos, mod_rm);
        return;
    }

    if (op0->kind == OPERAND_MEM && op1->kind == OPERAND_REG)
    {
        if (op0->displacement == 0) // Bytes: 0x88 | ModRM
        {
            uint8_t mod_rm = (uint8_t)((op1->reg_num << 3) | op0->reg_num);
            emit_1_byte(buffer_pos, 0x88);
            emit_1_byte(buffer_pos, mod_rm);
            return;
        }

        // Bytes: 0x88 | ModRM | disp8
        uint8_t mod_rm = (uint8_t)((1 << 6) | (op1->reg_num << 3) | op0->reg_num);
        emit_1_byte(buffer_pos, 0x88);
        emit_1_byte(buffer_pos, mod_rm);
        emit_1_byte(buffer_pos, (uint8_t) op0->displacement);
        return;
    }

    fprintf(stderr, "encode_mov: unknown operand combination\n");
    assert(0);
}

void encode_add(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos)
{
    uint8_t mod_rm = (uint8_t) ((3 << 6) | op0->reg_num);

    if (op0->reg_size == 1) // Bytes: 0x80 | ModRM(/0) | imm8
    {
        emit_1_byte(buffer_pos, 0x80);
        emit_1_byte(buffer_pos, mod_rm);
        emit_1_byte(buffer_pos, (uint8_t) op1->imm_value);
        return;
    }

    // Bytes: REX.W(0x48) | 0x83 | ModRM(/0) | imm8
    emit_1_byte(buffer_pos, 0x48);
    emit_1_byte(buffer_pos, 0x83);
    emit_1_byte(buffer_pos, mod_rm);
    emit_1_byte(buffer_pos, (uint8_t) op1->imm_value);
}

void encode_sub(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos)
{
    uint8_t mod_rm = (uint8_t) ((3 << 6) | (5 << 3) | op0->reg_num);

    if (op0->reg_size == 1) // Bytes: 0x80 | ModRM(/5) | imm8
    {
        emit_1_byte(buffer_pos, 0x80);
        emit_1_byte(buffer_pos, mod_rm);
        emit_1_byte(buffer_pos, (uint8_t) op1->imm_value);
        return;
    }

    // Bytes: REX.W(0x48) | 0x83 | ModRM(/5) | imm8
    emit_1_byte(buffer_pos, 0x48);
    emit_1_byte(buffer_pos, 0x83);
    emit_1_byte(buffer_pos, mod_rm);
    emit_1_byte(buffer_pos, (uint8_t) op1->imm_value);
}

void encode_cmp(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos)
{
    uint8_t mod_rm = (uint8_t) ((3 << 6) | (7 << 3) | op0->reg_num);

    if (op0->reg_size == 1) // Bytes: 0x80 | ModRM(/7) | imm8
    {
        emit_1_byte(buffer_pos, 0x80);
        emit_1_byte(buffer_pos, mod_rm);
        emit_1_byte(buffer_pos, (uint8_t) op1->imm_value);
        return;
    }

    // Bytes: REX.W(0x48) | 0x83 | ModRM(/7) | imm8
    emit_1_byte(buffer_pos, 0x48);
    emit_1_byte(buffer_pos, 0x83);
    emit_1_byte(buffer_pos, mod_rm);
    emit_1_byte(buffer_pos, (uint8_t) op1->imm_value);
}

void encode_inc(const operand_t* op, uint8_t** buffer_pos)
{
    // Bytes: REX.W(0x48) | 0xFF | ModRM(/0)
    uint8_t mod_rm = (uint8_t)((3 << 6) | op->reg_num);
    emit_1_byte(buffer_pos, 0x48);
    emit_1_byte(buffer_pos, 0xFF);
    emit_1_byte(buffer_pos, mod_rm);
}

void encode_dec(const operand_t* op, uint8_t** buffer_pos)
{
    // Bytes: REX.W(0x48) | 0xFF | ModRM(/1)
    uint8_t mod_rm = (uint8_t)((3 << 6) | (1 << 3) | op->reg_num);
    emit_1_byte(buffer_pos, 0x48);
    emit_1_byte(buffer_pos, 0xFF);
    emit_1_byte(buffer_pos, mod_rm);
}

void encode_div(const operand_t* op, uint8_t** buffer_pos)
{
    // Bytes: REX.W(0x48) | 0xF7 | ModRM(/6)
    uint8_t mod_rm = (uint8_t)((3 << 6) | (6 << 3) | op->reg_num);
    emit_1_byte(buffer_pos, 0x48);
    emit_1_byte(buffer_pos, 0xF7);
    emit_1_byte(buffer_pos, mod_rm);
}

void encode_lea(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos)
{
    // Bytes: REX.W(0x48) | 0x8D | ModRM | disp8
    uint8_t mod_rm = (uint8_t)((1 << 6) | (op0->reg_num << 3) | op1->reg_num);
    emit_1_byte(buffer_pos, 0x48);
    emit_1_byte(buffer_pos, 0x8D);
    emit_1_byte(buffer_pos, mod_rm);
    emit_1_byte(buffer_pos, (uint8_t) op1->displacement);
}

void encode_movzx(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos)
{
    // Bytes: REX.W(0x48) | 0x0F | 0xB6 | ModRM | disp8
    uint8_t mod_rm = (uint8_t)((1 << 6) | (op0->reg_num << 3) | op1->reg_num);
    emit_1_byte(buffer_pos, 0x48);
    emit_1_byte(buffer_pos, 0x0F);
    emit_1_byte(buffer_pos, 0xB6);
    emit_1_byte(buffer_pos, mod_rm);
    emit_1_byte(buffer_pos, (uint8_t) op1->displacement);
}

void encode_movsd(const operand_t* op0, const operand_t* op1,
                  const label_list_t* labels, uint64_t instruction_address, uint8_t** buffer_pos)
{
    if (op0->kind == OPERAND_XMM && op1->kind == OPERAND_MEM_REL)
    {
        // Bytes: 0xF2 | 0x0F | 0x10 | ModRM(rm = 101) | rel32(4)
        uint8_t mod_rm = (uint8_t) ((op0->reg_num << 3) | 5);
        uint64_t target = find_label_address(labels, op1->label_name);
        int32_t rel32 = target - (instruction_address + 8);
        emit_1_byte(buffer_pos, 0xF2);
        emit_1_byte(buffer_pos, 0x0F);
        emit_1_byte(buffer_pos, 0x10);
        emit_1_byte(buffer_pos, mod_rm);
        emit_4_bytes(buffer_pos, (uint32_t) rel32);
        return;
    }

    if (op0->kind == OPERAND_XMM && op1->kind == OPERAND_MEM)
    {
        // Bytes: 0xF2 | 0x0F | 0x10 | ModRM | disp8
        uint8_t mod_rm = (uint8_t) ((1 << 6) | (op0->reg_num << 3) | op1->reg_num);
        emit_1_byte(buffer_pos, 0xF2);
        emit_1_byte(buffer_pos, 0x0F);
        emit_1_byte(buffer_pos, 0x10);
        emit_1_byte(buffer_pos, mod_rm);
        emit_1_byte(buffer_pos, (uint8_t) op1->displacement);
        return;
    }

    if (op0->kind == OPERAND_MEM && op1->kind == OPERAND_XMM)
    {
        // Bytes: 0xF2 | 0x0F | 0x11 | ModRM | disp8
        uint8_t mod_rm = (uint8_t) ((1 << 6) | (op1->reg_num << 3) | op0->reg_num);
        emit_1_byte(buffer_pos, 0xF2);
        emit_1_byte(buffer_pos, 0x0F);
        emit_1_byte(buffer_pos, 0x11);
        emit_1_byte(buffer_pos, mod_rm);
        emit_1_byte(buffer_pos, (uint8_t)(int8_t) op0->displacement);
        return;
    }

    fprintf(stderr, "encode_movsd: unknown operand combination\n");
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

uint64_t find_label_address(const label_list_t* labels, const char* name)
{
    for (size_t i = 0; i < labels->count; i++)
    {
        if (!strcmp(labels->labels[i].name, name))
            return labels->labels[i].address;
    }

    fprintf(stderr, "Label not found: '%s'\n", name);
    assert(0);
    return 0;
}
