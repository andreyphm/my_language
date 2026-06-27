#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "tree_to_asm.h"
#include "font.h"

static const size_t TEXT_BUFFER_FIRST_SIZE = 1000;
static const size_t RODATA_BUFFER_FIRST_SIZE = 1000;

void tree_to_asm(node_t* tree, FILE* const output_file, const identifier_t* const identifiers)
{
    assert(tree);
    assert(output_file);
    assert(identifiers);

    context_t context =
    {
        .counters = {},
        .buffers = {}
    };

    initialize_buffers(&context.buffers);

    printf_to_buffer(&context.buffers.text,
                     ";*******************************************************;\n"
                     ";==================== PROGRAM START ====================;\n"
                     ";================== GitHub: andreyphm ==================;\n"
                     ";*******************************************************;\n\n"
                     "section .text\n\n");

    printf_to_buffer(&context.buffers.rodata,
                     "section .rodata\n\n"
                     "const_true:\n"
                     "\tdq 1.0\n"
                     "const_false:\n"
                     "\tdq 0.0\n");

    gen_prog(tree, identifiers, &context);

    printf_to_buffer(&context.buffers.text,
                     ";================= PROGRAM END =================;\n\n");

    buffers_to_file(&context.buffers, output_file);

    free_buffers(&context.buffers);

    printf(MAKE_BOLD_GREEN("Tree to NASM successful\n"));
}

void gen_prog(node_t* prog_node, const identifier_t* const identifiers, context_t* const context)
{
    assert(prog_node);
    assert(identifiers);
    assert(context);

    node_t* includes = prog_node->children[0];
    for (size_t i = 0; i < includes->child_count; i++)
        gen_include(includes->children[i], identifiers, context);

    node_t* functions = prog_node->children[1];
    for (size_t i = 0; i < functions->child_count; i++)
        gen_func(functions->children[i], identifiers, context);
}

void gen_include(node_t* include_node, const identifier_t* const identifiers, context_t* context)
{
    assert(include_node);
    assert(identifiers);
    assert(context);

    int library_id = include_node->data_t.include.id_number;
    const char* library_name = identifiers[library_id].name;

    if (!strcmp(library_name, "my_stdlib"))
    {
        FILE* stdlib_file = fopen("my_stdlib.asm", "r");
        struct stat file_stat = {};
        fstat(fileno(stdlib_file), &file_stat);
        size_t file_size = (size_t) file_stat.st_size;

        char* stdlib_buffer = (char*) calloc(file_size + 1, sizeof(char));
        fread(stdlib_buffer, sizeof(char), file_size, stdlib_file);
        fclose(stdlib_file);

        printf_to_buffer(&context->buffers.text, "%s", stdlib_buffer);
        free(stdlib_buffer);

        printf_to_buffer(&context->buffers.rodata,
                         "__stdlib_neg0:\n"
                         "\tdq -0.0\n"
                         "__stdlib_1m:\n"
                         "\tdq 1000000.0\n"
                         "__stdlib_10:\n"
                         "\tdq 10.0\n"
                         "__stdlib_01:\n"
                         "\tdq 0.1\n");
    }
    else
        fprintf(stderr, MAKE_BOLD_RED("Unknown library: %s\n"), library_name);
}

void gen_func(node_t* func_node, const identifier_t* const identifiers, context_t* context)
{
    assert(func_node);
    assert(identifiers);
    assert(context);

    size_t func_id = (size_t)func_node->data_t.function.id_number;
    context->counters.current_func_id = func_id;
    context->counters.stack_offset = 0;
    size_t frame_size = align_up_16(func_node->data_t.function.frame_size);
    const char* func_name = identifiers[func_node->data_t.function.id_number].name;

    if (!strcmp(func_name, "main"))
        printf_to_buffer(&context->buffers.text, "main:\n");

    printf_to_buffer(&context->buffers.text,
                     ";==================== FUNCTION \"%s\" ====================;\n"
                     "func_%zu:\n"
                     "\tpush rbp\n"
                     "\tmov rbp, rsp\n"
                     "\tsub rsp, %zu\t\t\t\t\t; Stack preparation\n\n",
                     func_name, func_id, frame_size);
    
    node_t* args_node = func_node->children[0];
    for (size_t i = 0; i < args_node->child_count; i++)
    {
        printf_to_buffer(&context->buffers.text,
                         "\tmovsd xmm0, [rbp + %zu]\n"
                         "\tmovsd [rbp - %zu], xmm0\t\t; Take argument %zu\n\n",
                         (i + 2) * sizeof(double), (i + 1) * sizeof(double), i + 1);
    }

    gen_block(func_node->children[1], identifiers, context);

    printf_to_buffer(&context->buffers.text,
                     "func_end_%zu:\n"
                     "\tadd rsp, %zu\n"
                     "\tpop rbp\n",
                     func_id, frame_size);

    if (!strcmp(func_name, "main"))
    {
        printf_to_buffer(&context->buffers.text,
                         "\tcall __exit\n\n");
    }
    else
    {
        printf_to_buffer(&context->buffers.text,
                         "\tret\n\n");
    }
}

void gen_block(node_t* block_node, const identifier_t* const identifiers, context_t* context)
{
    assert(block_node);
    assert(identifiers);
    assert(context);

    if (block_node->child_count <= 0) return;

    for (size_t i = 0; i < block_node->child_count; i++)
        gen_op(block_node->children[i], identifiers, context);
}

void gen_op(node_t* op_node, const identifier_t* const identifiers, context_t* context)
{
    assert(op_node);
    assert(identifiers);
    assert(context);

    switch (op_node->kind)
    {
        case NODE_RET:
            gen_ret(op_node, identifiers, context);
            break;

        case NODE_VAR_DECL:
            if (op_node->child_count >= 2)
                gen_var_decl(op_node, identifiers, context);
            break;

        case NODE_IF:
            gen_if(op_node, identifiers, context);
            break;

        case NODE_WHILE:
            gen_while(op_node, identifiers, context);
            break;

        case NODE_BREAK:
            gen_break(context);
            break;

        case NODE_OP:
            op_node_to_asm(op_node, identifiers, context);
            break;

        case NODE_CALL:
            gen_call(op_node, identifiers, context);
            break;

        default:
            break;
    }
}

void gen_if(node_t* if_node, const identifier_t* const identifiers, context_t* context)
{
    assert(if_node);
    assert(identifiers);
    assert(context);

    size_t if_id = ++context->counters.if_counter;

    printf_to_buffer(&context->buffers.text,
                     ";==================== IF_%zu ====================;\n", if_id);

    gen_expr(if_node->children[0], identifiers, context);
    printf_to_buffer(&context->buffers.text,
                     "\tucomisd xmm0, [rel const_false]\t; Compare xmm0 with 0.0 (false)\n"
                     "\tje .if_end_%zu\n\n", if_id);

    gen_block(if_node->children[1], identifiers, context);

    if (if_node->child_count >= 3)
    {
        printf_to_buffer(&context->buffers.text,
                         "\tjmp .if_else_end_%zu\n\n",
                         if_id);
    }

    printf_to_buffer(&context->buffers.text,
                     ".if_end_%zu:\n",
                     if_id);

    if (if_node->child_count >= 3)
    {
        printf_to_buffer(&context->buffers.text,
                         ";==================== ELSE_%zu ====================;\n",
                         if_id);

        gen_block(if_node->children[2], identifiers, context);

        printf_to_buffer(&context->buffers.text,
                         ".if_else_end_%zu:\n\n",
                         if_id);
    }
}

void gen_while(node_t* while_node, const identifier_t* const identifiers, context_t* context)
{
    assert(while_node);
    assert(identifiers);
    assert(context);

    size_t while_id = ++context->counters.while_counter;
    context->counters.while_stack_counter++;

    printf_to_buffer(&context->buffers.text,
                     ";==================== WHILE_%zu ====================;\n",
                     while_id); 

    gen_expr(while_node->children[0], identifiers, context);
    printf_to_buffer(&context->buffers.text,
                     "\tucomisd xmm0, [rel const_false]\t; Compare xmm0 with 0.0 (false)\n"
                     "\tje .while_end_%zu\n"
                     ".while_loop_%zu:\n",
                     while_id, while_id);

    gen_block(while_node->children[1], identifiers, context);

    gen_expr(while_node->children[0], identifiers, context);
    printf_to_buffer(&context->buffers.text,
                     "\tucomisd xmm0, [rel const_false]\t; Compare xmm0 with 0.0 (false)\n"
                     "\tjne .while_loop_%zu\n"
                     ".while_end_%zu:\n\n",
                     while_id, while_id);
}

void gen_break(context_t* context)
{
    assert(context);

    printf_to_buffer(&context->buffers.text,
                     ";==================== BREAK ====================;\n"
                     "\tjmp .while_end_%zu\n\n",
                     context->counters.while_stack_counter);

    context->counters.while_stack_counter--;
}

void gen_var_decl(node_t* var_decl_node, const identifier_t* const identifiers, context_t* context)
{
    assert(var_decl_node);
    assert(identifiers);
    assert(context);

    node_t* var_node = var_decl_node->children[0];
    printf_to_buffer(&context->buffers.text,
                     ";==================== VAR_DECL_ID %d \"%s\" ====================;\n",
                     var_node->data_t.variable.unique_id,
                     identifiers[var_node->data_t.variable.id_number].name);

    gen_expr(var_decl_node->children[1], identifiers, context);

    printf_to_buffer(&context->buffers.text,
                     "\tmovsd [rbp - %zu], xmm0\t\t; variable_%d initialize\n\n",
                     var_decl_node->data_t.variable.stack_offset,
                     var_node->data_t.variable.unique_id);
}

void gen_ret(node_t* ret_node, const identifier_t* const identifiers, context_t* context)
{
    assert(ret_node);
    assert(identifiers);
    assert(context);

    printf_to_buffer(&context->buffers.text,
                     ";==================== RET ====================;\n");

    if (ret_node->child_count >= 1)
        gen_expr(ret_node->children[0], identifiers, context);
    else
    {
        printf_to_buffer(&context->buffers.text,
                         "\txorpd xmm0, xmm0\n");
    }

    printf_to_buffer(&context->buffers.text,
                     "\tjmp func_end_%zu\n\n",
                     context->counters.current_func_id);
}

void gen_expr(node_t* expr_node, const identifier_t* const identifiers, context_t* context)
{
    assert(expr_node);
    assert(identifiers);
    assert(context);

    switch(expr_node->kind)
    {
        case NODE_NUM:
            gen_num(expr_node, context);
            break;

        case NODE_VAR:
            printf_to_buffer(&context->buffers.text,
                             "\tmovsd xmm0, [rbp - %zu]\n",
                             expr_node->data_t.variable.stack_offset);
            break;

        case NODE_OP:
            op_node_to_asm(expr_node, identifiers, context);
            break;

        case NODE_CALL:
            gen_call(expr_node, identifiers, context);
            break;

        default:
            break;
    }
}

void gen_num(node_t* num_node, context_t* context)
{
    assert(num_node);
    assert(context);

    printf_to_buffer(&context->buffers.text,
                     "\tmovsd xmm0, [rel const_%zu]\n",
                     context->counters.const_counter);

    printf_to_buffer(&context->buffers.rodata,
                     "const_%zu:\n"
                     "\tdq %#.17g\n",
                     context->counters.const_counter,
                     num_node->data_t.number);

    context->counters.const_counter++;
}

void gen_call(node_t* call_node, const identifier_t* const identifiers, context_t* context)
{
    assert(call_node);
    assert(identifiers);
    assert(context);

    node_t* args_node = call_node->children[0];
    int function_id = call_node->data_t.function.id_number;

    if (!strcmp(identifiers[function_id].name, "out"))
    {
        gen_out(call_node, identifiers, context);
        return;
    }

    if (!strcmp(identifiers[function_id].name, "in"))
    {
        gen_in(call_node, identifiers, context);
        return;
    }

    bool aligned = align_stack_before_call(context);

    if (args_node->child_count >= 1)
    {
        printf_to_buffer(&context->buffers.text,
                         ";================= CALL \"%s\" =================;\n",
                         identifiers[function_id].name);
        gen_sub_rsp(context, args_node->child_count * sizeof(double));

        for (size_t i = 0; i < args_node->child_count; i++)
        {
            gen_expr(args_node->children[i], identifiers, context);
            printf_to_buffer(&context->buffers.text,
                             "\tmovsd [rsp + %zu], xmm0\t\t; Save func argument %zu\n\n",
                             i * sizeof(double), i + 1);
        }
    }

    printf_to_buffer(&context->buffers.text,
                     "\tcall func_%zu\n\n",
                     function_id);

    if (args_node->child_count >= 1)
        gen_add_rsp(context, args_node->child_count * sizeof(double));

    unalign_stack_after_call(context, aligned);
}

void gen_out(node_t* out_node, const identifier_t* const identifiers, context_t* context)
{
    assert(out_node);
    assert(identifiers);
    assert(context);

    node_t* args_node = out_node->children[0];

    if (args_node->child_count != 1)
    {
        fprintf(stderr, MAKE_BOLD_RED("Error: out() expects only one argument\n"));
        return;
    }

    printf_to_buffer(&context->buffers.text,
                     ";==================== OUT ====================;\n");

    gen_expr(args_node->children[0], identifiers, context);

    bool aligned = align_stack_before_call(context);
    printf_to_buffer(&context->buffers.text,
                     "\tcall __out\n\n");
    unalign_stack_after_call(context, aligned);
}

void gen_in(node_t* in_node, const identifier_t* const identifiers, context_t* context)
{
    assert(in_node);
    assert(identifiers);
    assert(context);

    node_t* args_node = in_node->children[0];

    if (args_node->child_count != 0)
    {
        fprintf(stderr, MAKE_BOLD_RED("Error: in() expects no arguments\n"));
        return;
    }

    printf_to_buffer(&context->buffers.text,
                     ";==================== IN ====================;\n");
    bool aligned = align_stack_before_call(context);
    printf_to_buffer(&context->buffers.text,
                     "\tcall __in\n\n");
    unalign_stack_after_call(context, aligned);
}

void op_node_to_asm(node_t* op_node, const identifier_t* const identifiers, context_t* context)
{
    assert(op_node);
    assert(identifiers);
    assert(context);

    gen_expr(op_node->children[1], identifiers, context);

    switch(op_node->data_t.op)
    {
        case ADD:
            gen_add(op_node, identifiers, context);
            break;

        case SUB:
            gen_sub(op_node, identifiers, context);
            break;

        case MUL:
            gen_mul(op_node, identifiers, context);
            break;

        case DIV:
            gen_div(op_node, identifiers, context);
            break;

        case ASSIGN:
            printf_to_buffer(&context->buffers.text,
                             "\tmovsd [rbp - %zu], xmm0\n",
                             op_node->children[0]->data_t.variable.stack_offset);
            break;

        case IS_EQUAL:
            gen_cmp(op_node, identifiers, context, "je");
            break;

        case IS_NOT_EQUAL:
            gen_cmp(op_node, identifiers, context, "jne");
            break;

        case GREATER_EQUAL:
            gen_cmp(op_node, identifiers, context, "jae");
            break;

        case GREATER:
            gen_cmp(op_node, identifiers, context, "ja");
            break;

        case LESS_EQUAL:
            gen_cmp(op_node, identifiers, context, "jbe");
            break;

        case LESS:
            gen_cmp(op_node, identifiers, context, "jb");
            break;

        default:
            break;
    }

    // printf_to_buffer(&context->buffers.text,
    //                  "\t\t\t; Operation complete\n\n");
}

void gen_add(node_t* add_node, const identifier_t* const identifiers, context_t* context)
{
    assert(add_node);
    assert(identifiers);
    assert(context);

    gen_sub_rsp(context, sizeof(double));
    printf_to_buffer(&context->buffers.text,
                     "\tmovsd [rsp], xmm0\t\t\t; Save temporary value\n\n");

    gen_expr(add_node->children[0], identifiers, context);

    printf_to_buffer(&context->buffers.text,
                     "\taddsd xmm0, [rsp]\n");
    gen_add_rsp(context, sizeof(double));
}

void gen_sub(node_t* sub_node, const identifier_t* const identifiers, context_t* context)
{
    assert(sub_node);
    assert(identifiers);
    assert(context);

    gen_sub_rsp(context, sizeof(double));
    printf_to_buffer(&context->buffers.text,
                     "\tmovsd [rsp], xmm0\t\t\t; Save temporary value\n\n");

    gen_expr(sub_node->children[0], identifiers, context);

    printf_to_buffer(&context->buffers.text,
                     "\tsubsd xmm0, [rsp]\n");
    gen_add_rsp(context, sizeof(double));
}

void gen_mul(node_t* mul_node, const identifier_t* const identifiers, context_t* context)
{
    assert(mul_node);
    assert(identifiers);
    assert(context);

    gen_sub_rsp(context, sizeof(double));
    printf_to_buffer(&context->buffers.text,
                     "\tmovsd [rsp], xmm0\t\t\t; Save temporary value\n\n");

    gen_expr(mul_node->children[0], identifiers, context);

    printf_to_buffer(&context->buffers.text,
                     "\tmulsd xmm0, [rsp]\n");
    gen_add_rsp(context, sizeof(double));
}

void gen_div(node_t* div_node, const identifier_t* const identifiers, context_t* context)
{
    assert(div_node);
    assert(identifiers);
    assert(context);

    gen_sub_rsp(context, sizeof(double));
    printf_to_buffer(&context->buffers.text,
                     "\tmovsd [rsp], xmm0\t\t\t; Save temporary value\n\n");

    gen_expr(div_node->children[0], identifiers, context);

    printf_to_buffer(&context->buffers.text,
                     "\tdivsd xmm0, [rsp]\n");
    gen_add_rsp(context, sizeof(double));
}

void gen_cmp(node_t* cmp_node, const identifier_t* const identifiers, context_t* context, const char* jump_word)
{
    assert(cmp_node);
    assert(identifiers);
    assert(context);
    assert(jump_word);

    size_t cmp_id = ++context->counters.cmp_counter;
    gen_sub_rsp(context, sizeof(double));
    printf_to_buffer(&context->buffers.text,
                     "\tmovsd [rsp], xmm0\t\t\t; Save temporary value\n\n");

    gen_expr(cmp_node->children[0], identifiers, context);

    printf_to_buffer(&context->buffers.text,
                     "\tucomisd xmm0, [rsp]\n"
                     "\t%s .cmp_true_%zu\n\n"
                     "\tmovsd xmm0, [rel const_false]\n"
                     "\tjmp .cmp_end_%zu\n\n"
                     ".cmp_true_%zu:\n"
                     "\tmovsd xmm0, [rel const_true]\n\n"
                     ".cmp_end_%zu:\n",
                     jump_word, cmp_id, cmp_id, cmp_id, cmp_id);
    gen_add_rsp(context, sizeof(double));
}

void gen_add_rsp(context_t* context, size_t bytes)
{
    assert(context);

    printf_to_buffer(&context->buffers.text, "\tadd rsp, %zu\n", bytes);
    context->counters.stack_offset -= bytes;
}

void gen_sub_rsp(context_t* context, size_t bytes)
{
    assert(context);

    printf_to_buffer(&context->buffers.text, "\tsub rsp, %zu\n", bytes);
    context->counters.stack_offset += bytes;
}

bool align_stack_before_call(context_t* context)
{
    assert(context);

    if (context->counters.stack_offset % 16 != 0)
    {
        printf_to_buffer(&context->buffers.text,
                         "\tsub rsp, 8\t\t\t\t\t; Stack alignment before call\n");
        context->counters.stack_offset += 8;
        return true;
    }

    return false;
}

void unalign_stack_after_call(context_t* context, bool was_aligned)
{
    assert(context);

    if (was_aligned)
    {
        printf_to_buffer(&context->buffers.text,
                         "\tadd rsp, 8\t\t\t\t\t; Remove alignment\n");
        context->counters.stack_offset -= 8;
    }
}

size_t align_up_16(size_t number)
{
    return (number + 15) / 16 * 16;
}

void printf_to_buffer(buffer_data_t* buffer_data, const char* format, ...)
{
    assert(buffer_data);

    va_list v_list = {};
    va_start(v_list, format);
    va_list v_list_copy = {};
    va_copy(v_list_copy, v_list);

    size_t available = buffer_data->capacity - buffer_data->pos;
    size_t needed = (size_t) vsnprintf(buffer_data->buffer + buffer_data->pos,
                                       buffer_data->capacity - buffer_data->pos, format, v_list);
    va_end(v_list);

    if (needed >= available)
    {
        size_t new_capacity = buffer_data->capacity;
        while (buffer_data->pos + needed >= new_capacity)
            new_capacity *= 2;

        char* new_buffer = (char*) realloc(buffer_data->buffer, new_capacity);
        assert(new_buffer);

        buffer_data->buffer = new_buffer;
        buffer_data->capacity = new_capacity;

        vsnprintf(buffer_data->buffer + buffer_data->pos,
                  buffer_data->capacity - buffer_data->pos, format, v_list_copy);
    }

    va_end(v_list_copy);
    buffer_data->pos += needed;
}

void initialize_buffers(buffers_t* buffers)
{
    assert(buffers);

    buffers->text.capacity    = TEXT_BUFFER_FIRST_SIZE;
    buffers->rodata.capacity  = RODATA_BUFFER_FIRST_SIZE;

    buffers->text.buffer    = (char*) calloc(buffers->text.capacity, sizeof(char));
    buffers->rodata.buffer  = (char*) calloc(buffers->rodata.capacity, sizeof(char));
}

void buffers_to_file(buffers_t* buffers, FILE* const output_file)
{
    assert(buffers);
    assert(output_file);

    fwrite(buffers->text.buffer, sizeof(char), buffers->text.pos, output_file);
    fwrite(buffers->rodata.buffer, sizeof(char), buffers->rodata.pos, output_file);
}

void free_buffers(buffers_t* buffers)
{
    assert(buffers);

    free(buffers->text.buffer);
    free(buffers->rodata.buffer);

    buffers->text.buffer = nullptr;
    buffers->rodata.buffer = nullptr;
}
