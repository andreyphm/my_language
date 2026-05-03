#ifndef BACK_END_H
#define BACK_END_H

#include "tree.h"

struct func_stack_frame
{
    size_t frame_size;
    
};

void back_end_run(node_t* tree, FILE* const output_file, const identifier_t* const identifiers);

void gen_prog(node_t* prog_node, const identifier_t* const identifiers);
void gen_func(node_t* func_node, const identifier_t* const identifiers);
void gen_block(node_t* block_node, const identifier_t* const identifiers);
void gen_op(node_t* op_node, const identifier_t* const identifiers);
void gen_expr(node_t* expr_node);

void op_node_to_asm(node_t* expr_node);

size_t count_local_vars(node_t* current);
size_t align_up_16(size_t number);

void printf_to_text_buffer(const char* format, ...);
void printf_to_rodata_buffer(const char* format, ...);

#endif // BACK_END_H
