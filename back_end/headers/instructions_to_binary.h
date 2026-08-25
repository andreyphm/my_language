#ifndef INSTRUCTIONS_TO_BINARY_H
#define INSTRUCTIONS_TO_BINARY_H

#include <stdio.h>
#include <stdint.h>
#include <elf.h>

#include "instructions.h"

#define SEGMENT_COUNT   2
#define BASE_VADDR      0x400000
#define CODE_OFFSET     (sizeof(Elf64_Ehdr) + SEGMENT_COUNT * sizeof(Elf64_Phdr))
#define ELF_PAGE_SIZE   0x1000

void instructions_to_binary(instruction_list_t* instruction_list, label_list_t* label_list,
                            FILE* binary_file, const char* stdlib_binary_path);

size_t get_instruction_size(const instruction_t* instruction);
void calculate_sizes(instruction_list_t* list);
void compute_labels_addresses(instruction_list_t* list, label_list_t* labels,
                              size_t rodata_instruction_index,
                              uint64_t text_start, uint64_t rodata_start);

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
void encode_lea(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos);
void encode_movzx(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos);
void encode_movsd(const operand_t* op0, const operand_t* op1,
                  const label_list_t* labels, uint64_t instruction_address, uint8_t** buffer_pos);
void encode_xorpd(const operand_t* op0, const operand_t* op1,
                  const label_list_t* labels, uint64_t instruction_address, uint8_t** buffer_pos);
void encode_ucomisd(const operand_t* op0, const operand_t* op1,
                    const label_list_t* labels, uint64_t instruction_address, uint8_t** buffer_pos);
void encode_cvttsd2si(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos);
void encode_cvtsi2sd(const operand_t* op0, const operand_t* op1, uint8_t** buffer_pos);
void encode_sse_arithmetic(uint8_t op_code, const operand_t* op0, const operand_t* op1,
                           const label_list_t* labels, uint64_t instruction_address, uint8_t** buffer_pos);
void encode_jmp(const operand_t* op, const label_list_t* labels, uint64_t instruction_address, uint8_t** buffer_pos);
void encode_call(const operand_t* op, const label_list_t* labels, uint64_t instruction_address, uint8_t** buffer_pos);
void encode_jcc(uint8_t op_code, const operand_t* op, const label_list_t* labels,
                uint64_t instruction_address, uint8_t** buffer_pos);

void emit_1_byte(uint8_t** buffer_pos, uint8_t byte);
void emit_4_bytes(uint8_t** buffer_pos, uint32_t value);
void emit_8_bytes(uint8_t** buffer_pos, uint64_t value);
uint64_t find_label_address(const label_list_t* labels, const char* name);

void fill_elf_header(Elf64_Ehdr* header, uint64_t entry_point);
void fill_program_headers(Elf64_Phdr* headers, uint64_t base_vaddr,
                          uint64_t text_file_size, uint64_t rodata_offset,
                          uint64_t rodata_size);

#endif // INSTRUCTIONS_TO_BINARY_H
