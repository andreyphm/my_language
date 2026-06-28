#ifndef ASM_TO_BINARY_H
#define ASM_TO_BINARY_H

#include <stdio.h>
#include <stdint.h>
#include <elf.h>

#include "back_end.h"

#define SEGMENT_COUNT   2
#define ENTRY_POINT     0x400000
#define BASE_VADDR      0x400000
#define CODE_OFFSET     sizeof(Elf64_Ehdr) + 2 * sizeof(Elf64_Phdr)

#define INITIAL_INSTRUCTIONS_CAPACITY 64

#define MAX_LABEL_NAME 64
#define INITIAL_LABELS_CAPACITY 16

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

enum section_kind
{
    NO_SECTION = 0,
    SECTION_TEXT,
    SECTION_RODATA
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

struct reg_info_t
{
    const char* name;
    size_t number;
    size_t size;
};

const reg_info_t registers_array[] =
{
    {"rax", 0, 8}, {"rcx", 1, 8}, {"rdx", 2, 8}, {"rbx", 3, 8},
    {"rsp", 4, 8}, {"rbp", 5, 8}, {"rsi", 6, 8}, {"rdi", 7, 8},
    {"eax", 0, 4}, {"ecx", 1, 4}, {"edx", 2, 4}, {"ebx", 3, 4},
    {"al",  0, 1}, {"cl",  1, 1}, {"dl",  2, 1}, {"bl",  3, 1}
};

const size_t REG_ARRAY_SIZE = sizeof(registers_array) / sizeof(registers_array[0]);

void asm_to_binary(FILE* const asm_file, FILE* const binary_file);
void asm_code_to_instructions(char* asm_buffer, instruction_list_t* const instruction_list, label_list_t* label_list);
void parse_instruction(char** asm_buffer, instruction_list_t* const instruction_list,
                                          label_list_t* label_list, section_kind* section);
void parse_operand(char** asm_buffer, operand_t* operand);
bool try_section(char** asm_buffer, section_kind* section);
bool try_label(char** asm_buffer, instruction_list_t* const instruction_list, label_list_t* label_list);
void read_mnemonic(char** asm_buffer, instruction_t* instruction);
bool try_double(char** asm_buffer, operand_t* operand);
bool try_imm(char** asm_buffer, operand_t* operand);
bool try_xmm(char** asm_buffer, operand_t* operand);
bool try_reg(char** asm_buffer, operand_t* operand);
bool try_mem(char** asm_buffer, operand_t* operand);
bool try_mem_rel(char** asm_buffer, operand_t* operand);
bool try_label_jump(char** asm_buffer, operand_t* operand);
void instruction_list_init(instruction_list_t* list);
void instruction_list_push_back(instruction_list_t* list, instruction_t instr);
void instruction_list_destroy(instruction_list_t* list);
void label_list_init(label_list_t* list);
void label_list_push_back(label_list_t* list, label_t label);
void label_list_destroy(label_list_t* list);

size_t get_instruction_size(const instruction_t* instruction);
void calculate_sizes(instruction_list_t* list);
void compute_labels_addresses(instruction_list_t* list, label_list_t* labels, uint64_t code_start);

size_t encode_all(const instruction_list_t* list, const label_list_t* labels, uint64_t code_start, uint8_t* buffer);
void encode_instruction(const instruction_t* instruction, const label_list_t* labels,
                              uint64_t instruction_address, uint8_t** buffer_pos);
void encode_syscall(uint8_t** buffer_pos);
void encode_ret(uint8_t** buffer_pos);
void encode_push(const operand_t* op, uint8_t** buffer_pos);
void encode_pop(const operand_t* op, uint8_t** buffer_pos);
void encode_dq(const operand_t* op, uint8_t** buffer_pos);
void encode_xor(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos);
void encode_test(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos);
void encode_mov(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos);
void encode_add(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos);
void encode_sub(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos);
void encode_cmp(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos);
void encode_inc(const operand_t* op, uint8_t** buffer_pos);
void encode_dec(const operand_t* op, uint8_t** buffer_pos);
void encode_div(const operand_t* op, uint8_t** buffer_pos);
void emit_1_byte(uint8_t** buffer_pos, uint8_t byte);
void emit_4_bytes(uint8_t** buffer_pos, uint32_t value);
void emit_8_bytes(uint8_t** buffer_pos, uint64_t value);

void fill_elf_header(Elf64_Ehdr* header, uint64_t entry_point);
void fill_program_header(Elf64_Phdr* header, uint64_t base_vaddr, uint64_t code_offset, uint64_t code_size,
                                                                  uint64_t data_offset, uint64_t data_size);

#endif // ASM_TO_BINARY_H
