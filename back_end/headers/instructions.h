#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stddef.h>
#include <stdint.h>

static const size_t MAX_LABEL_NAME = 64;

enum operand_kind
{
    NO_OPERAND,
    OPERAND_REG,
    OPERAND_XMM,
    OPERAND_MEM,
    OPERAND_MEM_REL,
    OPERAND_IMM,
    OPERAND_DOUBLE,
    OPERAND_LABEL
};

struct operand_t
{
    operand_kind kind;
    size_t reg_num;
    size_t reg_size;
    int64_t displacement;
    int64_t imm_value;
    double double_value;
    char label_name[64];
};

struct instruction_t
{
    char mnemonic[16];
    char comment[256];
    operand_t operands[2];
    size_t operand_count;
    size_t encoded_size;
};

struct instruction_list_t
{
    instruction_t* instructions;
    size_t count;
    size_t capacity;
};

struct label_t
{
    char name[MAX_LABEL_NAME];
    size_t instruction_index;
    uint64_t address;
};

struct label_list_t
{
    label_t* labels;
    size_t count;
    size_t capacity;
};

void instruction_list_init(instruction_list_t* list);
void instruction_list_push_back(instruction_list_t* list, instruction_t instruction);
void instruction_list_destroy(instruction_list_t* list);

void label_list_init(label_list_t* list);
void label_list_push_back(label_list_t* list, label_t label);
void label_list_destroy(label_list_t* list);

#endif // INSTRUCTIONS_H
