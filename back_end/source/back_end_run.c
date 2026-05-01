#include "back_end.h"
#include "font.h"

static size_t func_counter  = 0;
static size_t const_counter = 0;

static char text_buffer[10000] = "";
static size_t text_pos = 0;

static char rodata_buffer[10000] = "";
static size_t rodata_pos = 0;

void back_end_run(node_t* tree, FILE* const output_file, const identifier_t* const identifiers)
{
    text_pos += (size_t)snprintf(text_buffer, sizeof(text_buffer) - text_pos, "section .text\n\n");
    rodata_pos += (size_t)snprintf(rodata_buffer, sizeof(rodata_buffer) - rodata_pos, "\nsection .rodata\n\n");

    gen_prog(tree, identifiers);

    fwrite(text_buffer, sizeof(char), text_pos, output_file);
    fwrite(rodata_buffer, sizeof(char), rodata_pos, output_file);

    printf(MAKE_BOLD_GREEN("Tree to NASM successful\n"));
}

void gen_prog(node_t* prog_node, const identifier_t* const identifiers)
{
    for (size_t i = 0; i < prog_node->child_count; i++)
    {
        gen_func(prog_node->children[i], identifiers);
    }
}

void gen_func(node_t* func_node, const identifier_t* const identifiers)
{
    func_counter++;

    size_t body_local_vars = count_local_vars(func_node->children[1]);
    size_t local_vars_stack_size = (func_node->children[0]->child_count + body_local_vars) * sizeof(double);

    text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos,
                         ";========== FUNCTION \"%s\" ==========\n"
                         "func_%zu:\n"
                         "\tpush rbp\n"
                         "\tmov rbp, rsp\n"
                         "\tsub rsp, %zu\n",
                         identifiers[func_node->data_t.id_number].name,
                         func_counter,
                         align_up_16(local_vars_stack_size));

    gen_block(func_node->children[1], identifiers);

    // fprintf(output_file, ";==========RETURN %s==========\n"
    //                      "add rsp, %zu\n"
    //                      "ret\n\n",
    //                      identifiers[func_node->data_t.id_number].name,
    //                      align_up_16(local_vars_stack_size));
}

void gen_block(node_t* block_node, const identifier_t* const identifiers)
{
    if (block_node->child_count <= 0) return;

    for (size_t i = 0; i < block_node->child_count; i++)
        gen_op(block_node->children[i], identifiers);
}

void gen_op(node_t* op_node, const identifier_t* const identifiers)
{
    switch (op_node->kind)
    {
        case NODE_RET:
            if (op_node->child_count >= 1)
                gen_expr(op_node->children[0]);
            else
                text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos, "\txorpd xmm0, xmm0\n");
            return;

        default:
            return;
    }
}

void gen_expr(node_t* expr_node)
{
    switch(expr_node->kind)
    {
        case NODE_NUM:
            text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos,
                                 "\tmovsd xmm0, [rel const_%zu]\n", const_counter);
            rodata_pos += (size_t)snprintf(rodata_buffer + rodata_pos, sizeof(rodata_buffer) - rodata_pos,
                                   "const_%zu:\n"
                                   "\tdq %.17g\n", const_counter, expr_node->data_t.number);
            const_counter++;

        default:
            return;
    }
}

size_t count_local_vars(node_t* current)
{
    size_t local_vars = 0;
    if (current->child_count <= 0) return 0;
    
    for (size_t i = 0; i < current->child_count; i++)
        local_vars += count_local_vars(current->children[i]);

    if (current->kind == NODE_VAR_DECL) return 1;

    return local_vars;
}

size_t align_up_16(size_t number)
{
    return (number + 15) / 16 * 16;
}
