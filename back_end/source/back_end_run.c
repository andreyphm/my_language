#include "back_end.h"
#include "font.h"
#include "stdarg.h"

static const cond_op cond_op_array[] =
{
    {IS_EQUAL,       "je"},
    {IS_NOT_EQUAL,  "jne"},
    {GREATER_EQUAL, "jae"},
    {GREATER,        "ja"},
    {LESS_EQUAL,    "jbe"},
    {LESS,           "jb"}
};

static const size_t COND_OP_ARRAY_SIZE = sizeof(cond_op_array) / sizeof(cond_op_array[0]);

static size_t const_counter = 0;
static size_t cmp_counter = 0;
static size_t if_counter = 0;
static size_t while_counter = 0;
static size_t while_stack_counter = 0;
static size_t current_func_id = 0;

static char text_buffer[10000] = "";
static size_t text_pos = 0;

static char rodata_buffer[10000] = "";
static size_t rodata_pos = 0;

void back_end_run(node_t* tree, FILE* const output_file, const identifier_t* const identifiers)
{
    printf_to_text_buffer("section .text\n\n");

    printf_to_rodata_buffer("section .rodata\n\n"
                            "const_true:\n"
                            "\tdq 1.0\n"
                            "const_false:\n"
                            "\tdq 0.0\n");

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
    size_t func_id = (size_t)func_node->data_t.function.id_number;
    current_func_id = func_id;
    size_t frame_size = align_up_16(func_node->data_t.function.frame_size);

    printf_to_text_buffer(";========== FUNCTION \"%s\" ==========\n"
                          "func_%zu:\n"
                          "\tpush rbp\n"
                          "\tmov rbp, rsp\n"
                          "\tsub rsp, %zu\t\t; Stack preparation\n\n",
                          identifiers[func_node->data_t.function.id_number].name,
                          func_id,
                          frame_size);
    
    node_t* args_node = func_node->children[0];
    for (size_t i = 0; i < args_node->child_count; i++)
    {
        printf_to_text_buffer("\tmovsd xmm0, [rbp + %zu]\n"
                              "\tmovsd [rbp - %zu], xmm0\t\t; Take argument %zu\n\n",
                              (i + 2) * sizeof(double), (i + 1) * sizeof(double), i + 1);
    }

    gen_block(func_node->children[1], identifiers);

    printf_to_text_buffer("func_end_%zu:\n"
                          "\tadd rsp, %zu\n"
                          "\tpop rbp\n"
                          "\tret\t\t; Stack free\n\n",
                          func_id,
                          frame_size);
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
                gen_expr(op_node->children[0], identifiers);
            else
                printf_to_text_buffer("\txorpd xmm0, xmm0\n");

            printf_to_text_buffer("\tjmp func_end_%zu\n\n", current_func_id);
            break;

        case NODE_VAR_DECL:
            if (op_node->child_count >= 2)
            {
                node_t* var_node = op_node->children[0];
                printf_to_text_buffer(";========== VAR_DECL_ID %d \"%s\" ==========\n",
                                      var_node->data_t.variable.unique_id, identifiers[var_node->data_t.variable.id_number].name);

                gen_expr(op_node->children[1], identifiers);

                printf_to_text_buffer("\tmovsd [rbp - %zu], xmm0\t\t; variable_%d init\n\n",
                                      op_node->data_t.variable.stack_offset,
                                      var_node->data_t.variable.unique_id);
            }
            break;

        case NODE_IF:
        {
            size_t if_id = ++if_counter;
            printf_to_text_buffer(";========== IF_%zu ==========\n", if_id);

            gen_expr(op_node->children[0], identifiers);

            printf_to_text_buffer("\tucomisd xmm0, [rel const_false]\n"
                                  "\tje .if_end_%zu\n\n", if_id);

            gen_block(op_node->children[1], identifiers);

            if (op_node->child_count >= 3)
                printf_to_text_buffer("\tjmp .if_else_end_%zu\n\n", if_id);

            printf_to_text_buffer(".if_end_%zu:\n", if_id);

            if (op_node->child_count >= 3)
            {
                printf_to_text_buffer(";========== ELSE_%zu ==========\n", if_id);

                gen_block(op_node->children[2], identifiers);

                printf_to_text_buffer(".if_else_end_%zu:\n\n", if_id);
            }
        }
            break;

        case NODE_WHILE:
        {
            size_t while_id = ++while_counter;
            while_stack_counter++;
            printf_to_text_buffer(";========== WHILE_%zu ==========\n", while_id); 

            gen_expr(op_node->children[0], identifiers);
            printf_to_text_buffer("\tucomisd xmm0, [rel const_false]\n"
                                  "\tje .while_end_%zu\n\n"
                                  ".while_loop_%zu:\n",
                                  while_id, while_id);

            gen_block(op_node->children[1], identifiers);

            gen_expr(op_node->children[0], identifiers);
            printf_to_text_buffer("\tucomisd xmm0, [rel const_false]\n"
                                  "\tjne .while_loop_%zu\n\n"
                                  ".while_end_%zu:\n\n",
                                  while_id, while_id);
            break;
        }

        case NODE_BREAK:
            printf_to_text_buffer(";========== BREAK ==========\n"
                                  "\tjmp .while_end_%zu\n\n", while_stack_counter);
            while_stack_counter--;
            break;

        case NODE_OP:
            op_node_to_asm(op_node, identifiers);
            break;

        default:
            break;
    }
}

void gen_expr(node_t* expr_node, const identifier_t* const identifiers)
{
    switch(expr_node->kind)
    {
        case NODE_NUM:
            printf_to_text_buffer("\tmovsd xmm0, [rel const_%zu]\n", const_counter);

            printf_to_rodata_buffer("const_%zu:\n"
                                    "\tdq %#.17g\n",
                                    const_counter, expr_node->data_t.number);
            const_counter++;
            break;

        case NODE_VAR:
            printf_to_text_buffer("\tmovsd xmm0, [rbp - %zu]\n", expr_node->data_t.variable.stack_offset);
            break;

        case NODE_OP:
            op_node_to_asm(expr_node, identifiers);
            break;

        case NODE_CALL:
        {
            node_t* args_node = expr_node->children[0];
            if (args_node->child_count >= 1)
            {
                printf_to_text_buffer(";===== CALL \"%s\" =====\n"
                                      "\tsub rsp, %zu\n\n",
                                      identifiers[expr_node->data_t.function.id_number].name,
                                      args_node->child_count * sizeof(double));

                for (size_t i = 0; i < args_node->child_count; i++)
                {
                    gen_expr(args_node->children[i], identifiers);
                    printf_to_text_buffer("\tmovsd [rsp + %zu], xmm0\t\t; Save func argument %zu\n\n",
                                          i * sizeof(double), i + 1);
                }
            }
            printf_to_text_buffer("\tcall func_%zu\n\n"
                                  "\tadd rsp, %zu\n",
                                  expr_node->data_t.function.id_number,
                                  args_node->child_count * sizeof(double));
        }
            break;   

        default:
            break;
    }
}

void op_node_to_asm(node_t* expr_node, const identifier_t* const identifiers)
{
    gen_expr(expr_node->children[1], identifiers);

    switch(expr_node->data_t.op)
    {
        case ADD:
        {
            printf_to_text_buffer("\tsub rsp, %zu\n"
                                  "\tmovsd [rsp], xmm0\t\t; Save temporary value\n\n",
                                  sizeof(double));

            gen_expr(expr_node->children[0], identifiers);

            printf_to_text_buffer("\taddsd xmm0, [rsp]\n"
                                  "\tadd rsp, %zu",
                                  sizeof(double));
            break;
        }

        case SUB:
        {
            printf_to_text_buffer("\tsub rsp, %zu\n"
                                  "\tmovsd [rsp], xmm0\t\t; Save temporary value\n\n",
                                  sizeof(double));

            gen_expr(expr_node->children[0], identifiers);

            printf_to_text_buffer("\tsubsd xmm0, [rsp]\n"
                                  "\tadd rsp, %zu",
                                  sizeof(double));
            break;
        }

        case MUL:
        {
            printf_to_text_buffer("\tsub rsp, %zu\n"
                                  "\tmovsd [rsp], xmm0\t\t; Save temporary value\n\n",
                                  sizeof(double));

            gen_expr(expr_node->children[0], identifiers);

            printf_to_text_buffer("\tmulsd xmm0, [rsp]\n"
                                  "\tadd rsp, %zu",
                                  sizeof(double));
            break;
        }

        case DIV:
        {
            printf_to_text_buffer("\tsub rsp, %zu\n"
                                  "\tmovsd [rsp], xmm0\t\t; Save temporary value\n\n",
                                  sizeof(double));

            gen_expr(expr_node->children[0], identifiers);

            printf_to_text_buffer("\tdivsd xmm0, [rsp]\n"
                                  "\tadd rsp, %zu",
                                  sizeof(double));
            break;
        }

        case ASSIGN:
            printf_to_text_buffer("\tmovsd [rbp - %zu], xmm0",
                                  expr_node->children[0]->data_t.variable.stack_offset);
            break;

        case IS_EQUAL:
        case IS_NOT_EQUAL:
        case GREATER_EQUAL:
        case GREATER:
        case LESS_EQUAL:
        case LESS:
        {
            size_t cmp_id = ++cmp_counter;
            printf_to_text_buffer("\tsub rsp, %zu\n"
                                  "\tmovsd [rsp], xmm0\t\t; Save temporary value\n\n",
                                  sizeof(double));

            gen_expr(expr_node->children[0], identifiers);

            const char* jump_word = gen_jump_command(expr_node->data_t.op);

            printf_to_text_buffer("\tucomisd xmm0, [rsp]\n"
                                  "\t%s .cmp_true_%zu\n\n"
                                  "\tmovsd xmm0, [rel const_false]\n"
                                  "\tjmp .cmp_end_%zu\n\n"
                                  ".cmp_true_%zu:\n"
                                  "\tmovsd xmm0, [rel const_true]\n\n"
                                  ".cmp_end_%zu:\n"
                                  "\tadd rsp, %zu",
                                  jump_word, cmp_id, cmp_id, cmp_id, cmp_id, sizeof(double));
        }

        default:
            break;
    }

    printf_to_text_buffer("\t\t; Operation complete\n\n");
}

const char* gen_jump_command(operator_code op)
{
    for (size_t i = 0; i < COND_OP_ARRAY_SIZE; i++)
    {
        if (cond_op_array[i].op == op)
            return cond_op_array[i].jump_command;
    }

    return "UNKNOWN_JUMP_COMMAND";
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
