#include "back_end.h"
#include "font.h"

static size_t func_counter  = 0;
static size_t const_counter = 0;
static size_t local_vars_counter = 0;

static char text_buffer[10000] = "";
static size_t text_pos = 0;

static char rodata_buffer[10000] = "";
static size_t rodata_pos = 0;

void back_end_run(node_t* tree, FILE* const output_file, const identifier_t* const identifiers)
{
    text_pos += (size_t)snprintf(text_buffer, sizeof(text_buffer) - text_pos, "section .text\n\n");
    rodata_pos += (size_t)snprintf(rodata_buffer, sizeof(rodata_buffer) - rodata_pos, "section .rodata\n\n");

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
    local_vars_counter = func_node->children[0]->child_count + body_local_vars;
    size_t local_vars_stack_size = align_up_16(local_vars_counter * sizeof(double));

    text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos,
                         ";========== FUNCTION \"%s\" ==========\n"
                         "func_%zu:\n"
                         "\tpush rbp\n"
                         "\tmov rbp, rsp\n"
                         "\tsub rsp, %zu\t\t; Stack preparation\n\n",
                         identifiers[func_node->data_t.id_number].name,
                         func_counter,
                         local_vars_stack_size);

    gen_block(func_node->children[1], identifiers);

    text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos,
                         "\nfunc_end_%zu:\n"
                         "\tadd rsp, %zu\n"
                         "\tpop rbp\n"
                         "\tret\t\t; Stack free\n\n",
                         func_counter,
                         local_vars_stack_size);
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
            text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos,
                         ";========== RET ==========\n");
            if (op_node->child_count >= 1)
                gen_expr(op_node->children[0]);
            else
                text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos,
                                             "\txorpd xmm0, xmm0\n");
            text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos,
                                         "\tjmp func_end_%zu\n", func_counter);
            break;

        case NODE_VAR_DECL:
        {
            node_t* var_node = op_node->children[0];
            text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos,
                         ";========== VAR_DECL_ID %d ==========\n", var_node->unique_id);
            if (op_node->child_count >= 2)
            {
                gen_expr(op_node->children[1]);
                text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos,
                                             "\tmovsd [rbp - %zu], xmm0\t\t; variable_%d (\"%s\") init\n\n",
                                             (size_t)(op_node->unique_id + 1) * sizeof(double),
                                             var_node->unique_id, identifiers[var_node->data_t.id_number].name);
            }
        }

        default:
            break;
    }
}

void gen_expr(node_t* expr_node)
{
    switch(expr_node->kind)
    {
        case NODE_NUM:
            text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos,
                                         "\tmovsd xmm0, [rel const_%zu]\n",
                                         const_counter);
            rodata_pos += (size_t)snprintf(rodata_buffer + rodata_pos, sizeof(rodata_buffer) - rodata_pos,
                                   "const_%zu:\n"
                                   "\tdq %.17g\n", const_counter, expr_node->data_t.number);
            const_counter++;
            break;

        case NODE_VAR:
            text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos,
                                         "\tmovsd xmm0, [rbp - %zu]\n", (size_t)(expr_node->unique_id + 1) * sizeof(double));
            break;

        case NODE_OP:
            op_node_to_asm(expr_node);
            break;

        default:
            break;
    }
}

void op_node_to_asm(node_t* expr_node)
{
    gen_expr(expr_node->children[1]);
    text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos,
                                 "\tmovsd xmm1, xmm0\t\t; Save right value in xmm1\n\n");
    gen_expr(expr_node->children[0]);

    switch(expr_node->data_t.op)
    {
        case ADD:
            text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos,
                                         "\taddsd xmm0, xmm1");
            break;

        case SUB:
            text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos,
                                         "\tsubsd xmm0, xmm1");
            break;

        case MUL:
            text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos,
                                         "\tmulsd xmm0, xmm1");
            break;

        case DIV:
            text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos,
                                         "\tdivsd xmm0, xmm1");
            break;

        default:
            break;
    }

    text_pos += (size_t)snprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos,
                                         "\t\t; Operation end\n\n");
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
