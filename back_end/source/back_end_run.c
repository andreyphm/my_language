#include "back_end.h"
#include "font.h"
#include "stdarg.h"

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

    printf_to_text_buffer(";========== FUNCTION \"%s\" ==========\n"
                          "func_%zu:\n"
                          "\tpush rbp\n"
                          "\tmov rbp, rsp\n"
                          "\tsub rsp, %zu\t\t; Stack preparation\n\n",
                          identifiers[func_node->data_t.id_number].name,
                          func_counter,
                          local_vars_stack_size);

    gen_block(func_node->children[1], identifiers);

    printf_to_text_buffer("func_end_%zu:\n"
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
            printf_to_text_buffer(";========== RET ==========\n");

            if (op_node->child_count >= 1)
                gen_expr(op_node->children[0]);
            else
                printf_to_text_buffer("\txorpd xmm0, xmm0\n");

            printf_to_text_buffer("\tjmp func_end_%zu\n\n", func_counter);
            break;

        case NODE_VAR_DECL:
        {
            if (op_node->child_count >= 2)
            {
                node_t* var_node = op_node->children[0];
                printf_to_text_buffer(";========== VAR_DECL_ID %d \"%s\"==========\n",
                                      var_node->unique_id, identifiers[var_node->data_t.id_number].name);

                gen_expr(op_node->children[1]);
                printf_to_text_buffer("\tmovsd [rbp - %zu], xmm0\t\t; variable_%d init\n\n",
                                      (size_t)(op_node->unique_id + 1) * sizeof(double), var_node->unique_id);
            }

            break;
        }

        case NODE_OP:
            op_node_to_asm(op_node);
            break;

        default:
            break;
    }
}

void gen_expr(node_t* expr_node)
{
    switch(expr_node->kind)
    {
        case NODE_NUM:
            printf_to_text_buffer("\tmovsd xmm0, [rel const_%zu]\n",
                                  const_counter);

            printf_to_rodata_buffer("const_%zu:\n"
                                    "\tdq %.17g\n",
                                    const_counter, expr_node->data_t.number);
            const_counter++;
            break;

        case NODE_VAR:
            printf_to_text_buffer("\tmovsd xmm0, [rbp - %zu]\n", (size_t)(expr_node->unique_id + 1) * sizeof(double));
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

    switch(expr_node->data_t.op)
    {
        case ADD:
            printf_to_text_buffer("\tmovsd xmm1, xmm0\t\t; Save right value in xmm1\n");
            gen_expr(expr_node->children[0]);
            printf_to_text_buffer("\taddsd xmm0, xmm1");
            break;

        case SUB:
            printf_to_text_buffer("\tmovsd xmm1, xmm0\t\t; Save right value in xmm1\n");
            gen_expr(expr_node->children[0]);
            printf_to_text_buffer("\tsubsd xmm0, xmm1");
            break;

        case MUL:
            printf_to_text_buffer("\tmovsd xmm1, xmm0\t\t; Save right value in xmm1\n");
            gen_expr(expr_node->children[0]);
            printf_to_text_buffer("\tmulsd xmm0, xmm1");
            break;

        case DIV:
            printf_to_text_buffer("\tmovsd xmm1, xmm0\t\t; Save right value in xmm1\n");
            gen_expr(expr_node->children[0]);
            printf_to_text_buffer("\tdivsd xmm0, xmm1");
            break;

        case ASSIGN:
            printf_to_text_buffer("\tmovsd [rbp - %zu], xmm0",
                                  (size_t)(expr_node->children[0]->unique_id + 1) * sizeof(double));
            break;

        default:
            break;
    }

    printf_to_text_buffer("\t\t; Operation complete\n\n");
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

void printf_to_text_buffer(const char* format, ...)
{
    va_list v_list = {};
    va_start(v_list, format);
    text_pos += (size_t)vsnprintf(text_buffer + text_pos, sizeof(text_buffer) - text_pos, format, v_list);
    va_end(v_list);
}

void printf_to_rodata_buffer(const char* format, ...)
{
    va_list v_list = {};
    va_start(v_list, format);
    rodata_pos += (size_t)vsnprintf(rodata_buffer + rodata_pos, sizeof(rodata_buffer) - rodata_pos, format, v_list);
    va_end(v_list);
}
