#include <string.h>
#include <assert.h>

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

static char text_buffer[10000] = "";
static size_t text_pos = 0;

static char rodata_buffer[10000] = "";
static size_t rodata_pos = 0;

static char include_buffer[1000] = "";
static size_t include_pos = 0;

void back_end_run(node_t* tree, FILE* const output_file, const identifier_t* const identifiers)
{
    assert(tree);
    assert(output_file);
    assert(identifiers);

    printf_to_text_buffer(";*******************************************************;\n"
                          ";==================== PROGRAM START ====================;\n"
                          ";================== GitHub: andreyphm ==================;\n"
                          ";*******************************************************;\n\n"
                          "section .text\n\n");

    printf_to_rodata_buffer("section .rodata\n\n"
                            "const_true:\n"
                            "\tdq 1.0\n"
                            "const_false:\n"
                            "\tdq 0.0\n");
                        
    counters_t counters = {};

    gen_prog(tree, identifiers, &counters);

    printf_to_text_buffer(";================= PROGRAM END =================;\n\n");

    fwrite(include_buffer, sizeof(char), include_pos, output_file);
    fwrite(text_buffer, sizeof(char), text_pos, output_file);
    fwrite(rodata_buffer, sizeof(char), rodata_pos, output_file);

    printf(MAKE_BOLD_GREEN("Tree to NASM successful\n"));
}

void gen_prog(node_t* prog_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(prog_node);
    assert(identifiers);
    assert(counters);

    node_t* includes = prog_node->children[0];
    for (size_t i = 0; i < includes->child_count; i++)
        gen_include(includes->children[i], identifiers);

    node_t* functions = prog_node->children[1];
    for (size_t i = 0; i < functions->child_count; i++)
        gen_func(functions->children[i], identifiers, counters);
}

void gen_include(node_t* include_node, const identifier_t* const identifiers)
{
    assert(include_node);
    assert(identifiers);

    int library_id = include_node->data_t.include.id_number;
    const char* library_name = identifiers[library_id].name;

    if (!strcmp(library_name, "my_stdlib"))
        printf_to_include_buffer("%%include \"%s.asm\"\n", library_name);
    else
        fprintf(stderr, MAKE_BOLD_RED("Unknown library: %s\n"), library_name);
}

void gen_func(node_t* func_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(func_node);
    assert(identifiers);
    assert(counters);

    size_t func_id = (size_t)func_node->data_t.function.id_number;
    counters->current_func_id = func_id;
    size_t frame_size = align_up_16(func_node->data_t.function.frame_size);
    const char* func_name = identifiers[func_node->data_t.function.id_number].name;

    if (!strcmp(func_name, "main"))
    {
        printf_to_include_buffer("\nglobal main\n\n");
        printf_to_text_buffer("main:\n");
    }

    printf_to_text_buffer(";==================== FUNCTION \"%s\" ====================;\n"
                          "func_%zu:\n"
                          "\tpush rbp\n"
                          "\tmov rbp, rsp\n"
                          "\tsub rsp, %zu\t\t\t\t\t; Stack preparation\n\n",
                          func_name,
                          func_id,
                          frame_size);
    
    node_t* args_node = func_node->children[0];
    for (size_t i = 0; i < args_node->child_count; i++)
    {
        printf_to_text_buffer("\tmovsd xmm0, [rbp + %zu]\n"
                              "\tmovsd [rbp - %zu], xmm0\t\t; Take argument %zu\n\n",
                              (i + 2) * sizeof(double), (i + 1) * sizeof(double), i + 1);
    }

    gen_block(func_node->children[1], identifiers, counters);

    printf_to_text_buffer("func_end_%zu:\n"
                          "\tadd rsp, %zu\n"
                          "\tpop rbp\n"
                          "\tret\t\t\t\t\t\t\t; Stack free\n\n",
                          func_id,
                          frame_size);
}

void gen_block(node_t* block_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(block_node);
    assert(identifiers);
    assert(counters);

    if (block_node->child_count <= 0) return;

    for (size_t i = 0; i < block_node->child_count; i++)
        gen_op(block_node->children[i], identifiers, counters);
}

void gen_op(node_t* op_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(op_node);
    assert(identifiers);
    assert(counters);

    switch (op_node->kind)
    {
        case NODE_RET:
            gen_ret(op_node, identifiers, counters);
            break;

        case NODE_VAR_DECL:
            if (op_node->child_count >= 2)
                gen_var_decl(op_node, identifiers, counters);
            break;

        case NODE_IF:
            gen_if(op_node, identifiers, counters);
            break;

        case NODE_WHILE:
            gen_while(op_node, identifiers, counters);
            break;

        case NODE_BREAK:
            gen_break(counters);
            break;

        case NODE_OP:
            op_node_to_asm(op_node, identifiers, counters);
            break;

        case NODE_CALL:
            gen_call(op_node, identifiers, counters);
            break;

        default:
            break;
    }
}

void gen_if(node_t* if_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(if_node);
    assert(identifiers);
    assert(counters);

    size_t if_id = ++counters->if_counter;

    printf_to_text_buffer(";==================== IF_%zu ====================;\n", if_id);

    gen_expr(if_node->children[0], identifiers, counters);
    printf_to_text_buffer("\tucomisd xmm0, [rel const_false]\t; Compare xmm0 with 0.0 (false)\n"
                          "\tje .if_end_%zu\n\n", if_id);

    gen_block(if_node->children[1], identifiers, counters);

    if (if_node->child_count >= 3)
        printf_to_text_buffer("\tjmp .if_else_end_%zu\n\n", if_id);

    printf_to_text_buffer(".if_end_%zu:\n", if_id);

    if (if_node->child_count >= 3)
    {
        printf_to_text_buffer(";==================== ELSE_%zu ====================;\n", if_id);

        gen_block(if_node->children[2], identifiers, counters);

        printf_to_text_buffer(".if_else_end_%zu:\n\n", if_id);
    }
}

void gen_while(node_t* while_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(while_node);
    assert(identifiers);
    assert(counters);

    size_t while_id = ++counters->while_counter;
    counters->while_stack_counter++;
    printf_to_text_buffer(";==================== WHILE_%zu ====================;\n", while_id); 

    gen_expr(while_node->children[0], identifiers, counters);
    printf_to_text_buffer("\tucomisd xmm0, [rel const_false]\t; Compare xmm0 with 0.0 (false)\n"
                          "\tje .while_end_%zu\n"
                          ".while_loop_%zu:\n",
                          while_id, while_id);

    gen_block(while_node->children[1], identifiers, counters);

    gen_expr(while_node->children[0], identifiers, counters);
    printf_to_text_buffer("\tucomisd xmm0, [rel const_false]\t; Compare xmm0 with 0.0 (false)\n"
                          "\tjne .while_loop_%zu\n"
                          ".while_end_%zu:\n\n",
                          while_id, while_id);
}

void gen_break(counters_t* const counters)
{
    assert(counters);

    printf_to_text_buffer(";==================== BREAK ====================;\n"
                          "\tjmp .while_end_%zu\n\n",
                          counters->while_stack_counter);

    counters->while_stack_counter--;
}

void gen_var_decl(node_t* var_decl_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(var_decl_node);
    assert(identifiers);
    assert(counters);

    node_t* var_node = var_decl_node->children[0];
    printf_to_text_buffer(";==================== VAR_DECL_ID %d \"%s\" ====================;\n",
                            var_node->data_t.variable.unique_id,
                            identifiers[var_node->data_t.variable.id_number].name);

    gen_expr(var_decl_node->children[1], identifiers, counters);

    printf_to_text_buffer("\tmovsd [rbp - %zu], xmm0\t\t; variable_%d initialize\n\n",
                            var_decl_node->data_t.variable.stack_offset,
                            var_node->data_t.variable.unique_id);
}

void gen_ret(node_t* ret_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(ret_node);
    assert(identifiers);
    assert(counters);

    printf_to_text_buffer(";==================== RET ====================;\n");

    if (ret_node->child_count >= 1)
        gen_expr(ret_node->children[0], identifiers, counters);
    else
        printf_to_text_buffer("\txorpd xmm0, xmm0\n");

    printf_to_text_buffer("\tjmp func_end_%zu\n\n", counters->current_func_id);
}

void gen_expr(node_t* expr_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(expr_node);
    assert(identifiers);
    assert(counters);

    switch(expr_node->kind)
    {
        case NODE_NUM:
            gen_num(expr_node, counters);
            break;

        case NODE_VAR:
            printf_to_text_buffer("\tmovsd xmm0, [rbp - %zu]\n", expr_node->data_t.variable.stack_offset);
            break;

        case NODE_OP:
            op_node_to_asm(expr_node, identifiers, counters);
            break;

        case NODE_CALL:
            gen_call(expr_node, identifiers, counters);
            break;

        default:
            break;
    }
}

void gen_num(node_t* num_node, counters_t* const counters)
{
    assert(num_node);
    assert(counters);

    printf_to_text_buffer("\tmovsd xmm0, [rel const_%zu]\n", counters->const_counter);

    printf_to_rodata_buffer("const_%zu:\n"
                            "\tdq %#.17g\n",
                            counters->const_counter,
                            num_node->data_t.number);

    counters->const_counter++;
}

void gen_call(node_t* call_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(call_node);
    assert(identifiers);
    assert(counters);

    node_t* args_node = call_node->children[0];
    int function_id = call_node->data_t.function.id_number;

    if (!strcmp(identifiers[function_id].name, "out"))
    {
        gen_out(call_node, identifiers, counters);
        return;
    }

    if (!strcmp(identifiers[function_id].name, "in"))
    {
        gen_in(call_node, identifiers, counters);
        return;
    }

    if (args_node->child_count >= 1)
    {
        printf_to_text_buffer(";================= CALL \"%s\" =================;\n"
                              "\tsub rsp, %zu\n\n",
                              identifiers[function_id].name,
                              args_node->child_count * sizeof(double));

        for (size_t i = 0; i < args_node->child_count; i++)
        {
            gen_expr(args_node->children[i], identifiers, counters);
            printf_to_text_buffer("\tmovsd [rsp + %zu], xmm0\t\t; Save func argument %zu\n\n",
                                  i * sizeof(double), i + 1);
        }
    }

    printf_to_text_buffer("\tcall func_%zu\n\n"
                          "\tadd rsp, %zu\n",
                          function_id,
                          args_node->child_count * sizeof(double));
}

void gen_out(node_t* out_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(out_node);
    assert(identifiers);
    assert(counters);

    node_t* args_node = out_node->children[0];

    if (args_node->child_count != 1)
    {
        fprintf(stderr, MAKE_BOLD_RED("Error: out() expects only one argument\n"));
        return;
    }

    printf_to_text_buffer(";==================== OUT ====================;\n");

    gen_expr(args_node->children[0], identifiers, counters);

    printf_to_text_buffer("\tlea rdi, [rel __out_fmt]\t; Format string address is first argument of printf\n"
                          "\tmov al, 1\t\t\t\t\t; One double argument in xmm0\n"
                          "\tcall printf\n\n");
}

void gen_in(node_t* in_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(in_node);
    assert(identifiers);
    assert(counters);

    node_t* args_node = in_node->children[0];

    if (args_node->child_count != 0)
    {
        fprintf(stderr, MAKE_BOLD_RED("Error: in() expects no arguments\n"));
        return;
    }

    printf_to_text_buffer(";==================== IN ====================;\n"
                          "\tpush rbp\n"
                          "\tmov rbp, rsp\n"
                          "\tsub rsp, 16\t\t\t\t\t; Space for a double with alignment\n\n"
                          "\tlea rdi, [rel __in_fmt]\t\t; Format string address is first argument of scanf\n"
                          "\tlea rsi, [rbp - 8]\t\t\t; Write the address of the variable where scanf will store the value\n"
                          "\txor eax, eax\t\t\t\t; There is no xmm arguments\n"
                          "\tcall scanf\n"
                          "\tmovsd xmm0, [rbp - 8]\t\t; Save value in xmm0\n\n"
                          "\tadd rsp, 16\n"
                          "\tpop rsp\n");
}

void op_node_to_asm(node_t* op_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(op_node);
    assert(identifiers);
    assert(counters);

    gen_expr(op_node->children[1], identifiers, counters);

    switch(op_node->data_t.op)
    {
        case ADD:
            gen_add(op_node, identifiers, counters);
            break;

        case SUB:
            gen_sub(op_node, identifiers, counters);
            break;

        case MUL:
            gen_mul(op_node, identifiers, counters);
            break;

        case DIV:
            gen_div(op_node, identifiers, counters);
            break;

        case ASSIGN:
            printf_to_text_buffer("\tmovsd [rbp - %zu], xmm0",
                                  op_node->children[0]->data_t.variable.stack_offset);
            break;

        case IS_EQUAL:
        case IS_NOT_EQUAL:
        case GREATER_EQUAL:
        case GREATER:
        case LESS_EQUAL:
        case LESS:
            gen_cmp(op_node, identifiers, counters);
            break;

        default:
            break;
    }

    printf_to_text_buffer("       ; Operation complete\n\n");
}

void gen_add(node_t* add_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(add_node);
    assert(identifiers);
    assert(counters);

    printf_to_text_buffer("\tsub rsp, %zu\n"
                          "\tmovsd [rsp], xmm0\t\t\t; Save temporary value\n\n",
                          sizeof(double));

    gen_expr(add_node->children[0], identifiers, counters);

    printf_to_text_buffer("\taddsd xmm0, [rsp]\n"
                          "\tadd rsp, %zu",
                          sizeof(double));
}

void gen_sub(node_t* sub_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(sub_node);
    assert(identifiers);
    assert(counters);

    printf_to_text_buffer("\tsub rsp, %zu\n"
                          "\tmovsd [rsp], xmm0\t\t\t; Save temporary value\n\n",
                          sizeof(double));

    gen_expr(sub_node->children[0], identifiers, counters);

    printf_to_text_buffer("\tsubsd xmm0, [rsp]\n"
                          "\tadd rsp, %zu",
                          sizeof(double));
}

void gen_mul(node_t* mul_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(mul_node);
    assert(identifiers);
    assert(counters);

    printf_to_text_buffer("\tsub rsp, %zu\n"
                          "\tmovsd [rsp], xmm0\t\t\t; Save temporary value\n\n",
                          sizeof(double));

    gen_expr(mul_node->children[0], identifiers, counters);

    printf_to_text_buffer("\tmulsd xmm0, [rsp]\n"
                          "\tadd rsp, %zu",
                          sizeof(double));
}

void gen_div(node_t* div_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(div_node);
    assert(identifiers);
    assert(counters);

    printf_to_text_buffer("\tsub rsp, %zu\n"
                          "\tmovsd [rsp], xmm0\t\t\t; Save temporary value\n\n",
                          sizeof(double));

    gen_expr(div_node->children[0], identifiers, counters);

    printf_to_text_buffer("\tdivsd xmm0, [rsp]\n"
                          "\tadd rsp, %zu",
                          sizeof(double));
}

void gen_cmp(node_t* cmp_node, const identifier_t* const identifiers, counters_t* const counters)
{
    assert(cmp_node);
    assert(identifiers);
    assert(counters);

    size_t cmp_id = ++counters->cmp_counter;
    printf_to_text_buffer("\tsub rsp, %zu\n"
                          "\tmovsd [rsp], xmm0\t\t\t; Save temporary value\n\n",
                          sizeof(double));

    gen_expr(cmp_node->children[0], identifiers, counters);

    const char* jump_word = gen_jump_command(cmp_node->data_t.op);

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

void printf_to_include_buffer(const char* format, ...)
{
    va_list v_list = {};
    va_start(v_list, format);
    include_pos += (size_t)vsnprintf(include_buffer + include_pos, sizeof(include_buffer) - include_pos, format, v_list);
    va_end(v_list);
}
