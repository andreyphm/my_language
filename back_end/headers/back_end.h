#ifndef BACK_END_H
#define BACK_END_H

#include "tree.h"

struct cond_op
{
    operator_code op;
    const char* jump_command;
};

struct counters_t
{
    size_t const_counter;
    size_t cmp_counter;
    size_t if_counter;
    size_t while_counter;
    size_t while_stack_counter;
    size_t current_func_id;
};

void back_end_run(node_t* tree, FILE* const output_file, const identifier_t* const identifiers);

void gen_prog(node_t* prog_node, const identifier_t* const identifiers, counters_t* const counters);
void gen_func(node_t* func_node, const identifier_t* const identifiers, counters_t* const counters);
void gen_block(node_t* block_node, const identifier_t* const identifiers, counters_t* const counters);
void gen_op(node_t* op_node, const identifier_t* const identifiers, counters_t* const counters);
void gen_expr(node_t* expr_node, const identifier_t* const identifiers, counters_t* const counters);
void op_node_to_asm(node_t* expr_node, const identifier_t* const identifiers, counters_t* const counters);
const char* gen_jump_command(operator_code op);

size_t align_up_16(size_t number);

void printf_to_text_buffer(const char* format, ...);
void printf_to_rodata_buffer(const char* format, ...);

#endif // BACK_END_H
