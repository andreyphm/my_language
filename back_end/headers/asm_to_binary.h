#ifndef ASM_TO_BINARY_H
#define ASM_TO_BINARY_H

#include <stdio.h>
#include <stdint.h>  

#define SEGMENT_COUNT   2
#define ENTRY_POINT     0x400000
#define BASE_VADDR      0x400000
#define CODE_OFFSET     sizeof(Elf64_Ehdr) + 2 * sizeof(Elf64_Phdr)

#define INITIAL_INSTRUCTIONS_CAPACITY 64

#define MAX_LABEL_NAME 64
#define INITIAL_LABELS_CAPACITY 16

enum error_code
{
    NO_ERROR = 0,
    PARSE_ERROR,
    REALLOC_ERROR
};

enum operand_kind
{
    NO_OPERAND,
    OPERAND_REG,
    OPERAND_XMM,
    OPERAND_MEM,
    OPERAND_IMM,
    OPERAND_LABEL
};

struct operand_t
{
    operand_kind kind;
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
};

struct label_t
{
    char name[MAX_LABEL_NAME];
    size_t instruction_index;
};

struct label_list_t
{
    label_t* labels;
    size_t count;
    size_t capacity;
};

void asm_to_binary(FILE* const asm_file, FILE* const binary_file);
error_code asm_code_to_instructions(char* asm_buffer, instruction_list_t* const instruction_list, label_list_t* label_list);
error_code parse_instruction(char** asm_buffer, instruction_list_t* const instruction_list, label_list_t* label_list);

void instruction_list_init(instruction_list_t* list);
error_code instruction_list_push_back(instruction_list_t* list, instruction_t instr);
void instruction_list_destroy(instruction_list_t* list);

void label_list_init(label_list_t* list;
error_code label_list_push_back(label_list_t* list, label_t label);
void label_list_destroy(label_list_t* list);

void fill_elf_header(Elf64_Ehdr* header, uint64_t entry_point);
void fill_program_header(Elf64_Phdr* header, uint64_t base_vaddr, uint64_t code_offset, uint64_t code_size,
                                                                  uint64_t data_offset, uint64_t data_size);

#endif // ASM_TO_BINARY_H
