#ifndef TREE_TO_ASM_H
#define TREE_TO_ASM_H

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
    size_t stack_offset;
};

struct buffer_data_t
{
    char* buffer;
    size_t pos;
    size_t capacity;
};

struct buffers_t
{
    buffer_data_t text;
    buffer_data_t rodata;
    buffer_data_t include;
};

struct context_t
{
    counters_t counters;
    buffers_t buffers;
};

void tree_to_asm(node_t* tree, FILE* const output_file, const identifier_t* const identifiers);

void gen_prog(node_t* prog_node, const identifier_t* const identifiers, context_t* context);
void gen_include(node_t* include_node, const identifier_t* const identifiers, context_t* context);
void gen_func(node_t* func_node, const identifier_t* const identifiers, context_t* context);
void gen_block(node_t* block_node, const identifier_t* const identifiers, context_t* context);
void gen_op(node_t* op_node, const identifier_t* const identifiers, context_t* context);
void gen_if(node_t* if_node, const identifier_t* const identifiers, context_t* context);
void gen_while(node_t* while_node, const identifier_t* const identifiers, context_t* context);
void gen_break(context_t* context);
void gen_var_decl(node_t* var_decl_node, const identifier_t* const identifiers, context_t* context);
void gen_ret(node_t* ret_node, const identifier_t* const identifiers, context_t* context);
void gen_num(node_t* num_node, context_t* context);
void gen_expr(node_t* expr_node, const identifier_t* const identifiers, context_t* context);
void gen_call(node_t* call_node, const identifier_t* const identifiers, context_t* context);
void op_node_to_asm(node_t* op_node, const identifier_t* const identifiers, context_t* context);
void gen_add(node_t* add_node, const identifier_t* const identifiers, context_t* context);
void gen_sub(node_t* sub_node, const identifier_t* const identifiers, context_t* context);
void gen_mul(node_t* mul_node, const identifier_t* const identifiers, context_t* context);
void gen_div(node_t* div_node, const identifier_t* const identifiers, context_t* context);
void gen_cmp(node_t* cmp_node, const identifier_t* const identifiers, context_t* context, const char* jump_word);
void gen_out(node_t* out_node, const identifier_t* const identifiers, context_t* context);
void gen_in(node_t* in_node, const identifier_t* const identifiers, context_t* context);

void gen_sub_rsp(context_t* context, size_t bytes);
void gen_add_rsp(context_t* context, size_t bytes);
bool align_stack_before_call(context_t* context);
void unalign_stack_after_call(context_t* context, bool was_aligned);
size_t align_up_16(size_t number);

void printf_to_buffer(buffer_data_t* buffer_data, const char* format, ...);
void initialize_buffers(buffers_t* buffers);
void buffers_to_file(buffers_t* buffers, FILE* const output_file);
void free_buffers(buffers_t* buffers);

#endif // TREE_TO_ASM_H
