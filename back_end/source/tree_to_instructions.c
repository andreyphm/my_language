#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree_to_instructions.h"
#include "font.h"

struct instructions_counters_t
{
    size_t const_counter;
    size_t cmp_counter;
    size_t logic_counter;
    size_t if_counter;
    size_t while_counter;
    size_t current_func_id;
    size_t stack_offset;
};

struct instructions_context_t
{
    instructions_counters_t counters;
    instruction_list_t* instructions;
    label_list_t* labels;
    instruction_list_t rodata_instructions;
    label_list_t rodata_labels;
    size_t while_stack[64];
    size_t while_stack_size;
};

static void gen_prog(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_include(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_func(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_block(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_op(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_if(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_while(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_break(instructions_context_t* context);
static void gen_var_decl(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_ret(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_expr(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_num(node_t* node, instructions_context_t* context);
static void gen_call(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_out(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_in(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_sqrt(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_op_node(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_add(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_sub(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_mul(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_div(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_logic(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_logic_or(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_logic_and(node_t* node, const identifier_t* identifiers, instructions_context_t* context);
static void gen_cmp(node_t* node, const identifier_t* identifiers,
                    instructions_context_t* context, const char* jump_word);
static void gen_sub_rsp(instructions_context_t* context, size_t bytes);
static void gen_add_rsp(instructions_context_t* context, size_t bytes);
static bool align_stack_before_call(instructions_context_t* context);
static void unalign_stack_after_call(instructions_context_t* context, bool was_aligned);

static void emit_instruction(instructions_context_t* context, const instruction_t* instruction);
static void emit_rodata_instruction(instructions_context_t* context, const instruction_t* instruction);
static void emit_text_label(instructions_context_t* context, const char* name);
static void emit_rodata_label(instructions_context_t* context, const char* name);
static void emit_text_label_fmt(instructions_context_t* context, const char* format, ...);
static void emit_rodata_double(instructions_context_t* context, const char* label_name, double value);
static void get_or_add_rodata_double_label(instructions_context_t* context, double value,
                                           char* label_name, size_t label_name_size);
static void emit_comment(instructions_context_t* context, const char* format, ...);
static void comment_last_instruction(instructions_context_t* context, const char* format, ...);
static void emit_no_operands(instructions_context_t* context, const char* mnemonic);
static void emit_one_operand(instructions_context_t* context, const char* mnemonic, operand_t operand);
static void emit_two_operands(instructions_context_t* context, const char* mnemonic, operand_t first, operand_t second);

static operand_t make_reg(size_t reg_num, size_t reg_size);
static operand_t make_xmm(size_t reg_num);
static operand_t make_mem(size_t base_reg_num, int64_t displacement);
static operand_t make_mem_rel(const char* label_name);
static operand_t make_imm(int64_t value);
static operand_t make_double(double value);
static operand_t make_label(const char* label_name);
static operand_t make_label_fmt(const char* format, ...);

static operand_t rsp()  { return make_reg(4, 8); }
static operand_t rbp()  { return make_reg(5, 8); }
static operand_t xmm0() { return make_xmm(0); }

static void append_rodata(instructions_context_t* context);
static size_t align_up_16(size_t number);

void tree_to_instructions(node_t* tree, instruction_list_t* const instruction_list,
                          label_list_t* const label_list, const identifier_t* const identifiers)
{
    assert(tree);
    assert(instruction_list);
    assert(label_list);
    assert(identifiers);

    instruction_list_init(instruction_list);
    label_list_init(label_list);

    instructions_context_t context =
    {
        .counters = {},
        .instructions = instruction_list,
        .labels = label_list,
        .rodata_instructions = {},
        .rodata_labels = {},
    };

    instruction_list_init(&context.rodata_instructions);
    label_list_init(&context.rodata_labels);

    emit_rodata_double(&context, "const_true", 1.0);
    emit_rodata_double(&context, "const_false", 0.0);

    gen_prog(tree, identifiers, &context);

    append_rodata(&context);

    instruction_list_destroy(&context.rodata_instructions);
    label_list_destroy(&context.rodata_labels);

    printf(MAKE_BOLD_GREEN("Tree to instructions successful\n"));
}

void gen_prog(node_t* prog_node, const identifier_t* identifiers, instructions_context_t* context)
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

void gen_include(node_t* include_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(include_node);
    assert(identifiers);
    assert(context);

    int library_id = include_node->data_t.include.id_number;
    const char* library_name = identifiers[library_id].name;

    if (!strcmp(library_name, "my_stdlib"))
        return;
    else
    {
        fprintf(stderr, MAKE_BOLD_RED("Unknown library: %s\n"), library_name);
    }
}

void gen_func(node_t* func_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(func_node);
    assert(identifiers);
    assert(context);

    size_t func_id = (size_t) func_node->data_t.function.id_number;
    size_t frame_size = align_up_16(func_node->data_t.function.frame_size);
    const char* func_name = identifiers[func_node->data_t.function.id_number].name;

    context->counters.current_func_id = func_id;
    context->counters.stack_offset = 0;

    emit_comment(context, "==================== FUNCTION \"%s\" ====================", func_name);

    if (!strcmp(func_name, "теорема") || !strcmp(func_name, "main"))
        emit_text_label(context, "main");

    emit_text_label_fmt(context, "func_%zu", func_id);

    emit_one_operand(context, "push", rbp());
    emit_two_operands(context, "mov", rbp(), rsp());
    emit_two_operands(context, "sub", rsp(), make_imm((int64_t) frame_size));
    comment_last_instruction(context, "Stack preparation");

    node_t* args_node = func_node->children[0];
    for (size_t i = 0; i < args_node->child_count; i++)
    {
        emit_two_operands(context, "movsd", xmm0(), make_mem(5, (int64_t) ((i + 2) * sizeof(double))));
        emit_two_operands(context, "movsd", make_mem(5, -((int64_t) ((i + 1) * sizeof(double)))), xmm0());
        comment_last_instruction(context, "Take argument %zu", i + 1);
    }

    gen_block(func_node->children[1], identifiers, context);

    emit_text_label_fmt(context, "func_end_%zu", func_id);

    emit_two_operands(context, "add", rsp(), make_imm((int64_t) frame_size));
    emit_one_operand(context, "pop", rbp());

    if (!strcmp(func_name, "теорема") || !strcmp(func_name, "main"))
        emit_one_operand(context, "call", make_label("__exit"));
    else
        emit_no_operands(context, "ret");
}

void gen_block(node_t* block_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(block_node);
    assert(identifiers);
    assert(context);

    for (size_t i = 0; i < block_node->child_count; i++)
        gen_op(block_node->children[i], identifiers, context);
}

void gen_op(node_t* op_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(op_node);
    assert(identifiers);
    assert(context);

    switch (op_node->kind)
    {
        case NODE_VAR_DECL:
            if (op_node->child_count >= 2)
                return gen_var_decl(op_node, identifiers, context);
            break;

        case NODE_RET:      return gen_ret(op_node, identifiers, context);
        case NODE_IF:       return gen_if(op_node, identifiers, context);
        case NODE_WHILE:    return gen_while(op_node, identifiers, context);
        case NODE_BREAK:    return gen_break(context);
        case NODE_OP:       return gen_op_node(op_node, identifiers, context);
        case NODE_CALL:     return gen_call(op_node, identifiers, context);

        default:
            break;
    }
}

void gen_if(node_t* if_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(if_node);
    assert(identifiers);
    assert(context);

    size_t if_id = ++context->counters.if_counter;

    emit_comment(context, "==================== IF_%zu ====================", if_id);

    gen_expr(if_node->children[0], identifiers, context);
    emit_two_operands(context, "ucomisd", xmm0(), make_mem_rel("const_false"));
    comment_last_instruction(context, "Compare expression with false");
    emit_one_operand(context, "je", make_label_fmt(".if_end_%zu", if_id));

    gen_block(if_node->children[1], identifiers, context);

    if (if_node->child_count >= 3)
        emit_one_operand(context, "jmp", make_label_fmt(".if_else_end_%zu", if_id));

    emit_text_label_fmt(context, ".if_end_%zu", if_id);

    if (if_node->child_count >= 3)
    {
        emit_comment(context, "==================== ELSE_%zu ====================", if_id);
        gen_block(if_node->children[2], identifiers, context);
        emit_text_label_fmt(context, ".if_else_end_%zu", if_id);
    }
}

void gen_while(node_t* while_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(while_node);
    assert(identifiers);
    assert(context);

    size_t while_id = ++context->counters.while_counter;
    emit_comment(context, "==================== WHILE_%zu ====================", while_id);
    assert(context->while_stack_size < sizeof(context->while_stack) / sizeof(context->while_stack[0]));
    context->while_stack[context->while_stack_size++] = while_id;

    gen_expr(while_node->children[0], identifiers, context);
    emit_two_operands(context, "ucomisd", xmm0(), make_mem_rel("const_false"));
    comment_last_instruction(context, "Compare expression with false");
    emit_one_operand(context, "je", make_label_fmt(".while_end_%zu", while_id));

    emit_text_label_fmt(context, ".while_loop_%zu", while_id);

    gen_block(while_node->children[1], identifiers, context);

    gen_expr(while_node->children[0], identifiers, context);
    emit_two_operands(context, "ucomisd", xmm0(), make_mem_rel("const_false"));
    comment_last_instruction(context, "Compare expression with false");
    emit_one_operand(context, "jne", make_label_fmt(".while_loop_%zu", while_id));

    emit_text_label_fmt(context, ".while_end_%zu", while_id);
    context->while_stack_size--;
}

void gen_break(instructions_context_t* context)
{
    assert(context);

    emit_comment(context, "==================== BREAK ====================");
    assert(context->while_stack_size > 0);
    emit_one_operand(context, "jmp",
                     make_label_fmt(".while_end_%zu", context->while_stack[context->while_stack_size - 1]));
}

void gen_var_decl(node_t* var_decl_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(var_decl_node);
    assert(identifiers);
    assert(context);

    node_t* variable_node = var_decl_node->children[0];
    emit_comment(context, "==================== VAR_DECL_ID %d \"%s\" ====================",
                 variable_node->data_t.variable.unique_id,
                 identifiers[variable_node->data_t.variable.id_number].name);

    gen_expr(var_decl_node->children[1], identifiers, context);
    emit_two_operands(context, "movsd", make_mem(5, -((int64_t) var_decl_node->data_t.variable.stack_offset)), xmm0());
    comment_last_instruction(context, "Initialize variable_%d", variable_node->data_t.variable.unique_id);
}

void gen_ret(node_t* ret_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(ret_node);
    assert(identifiers);
    assert(context);

    emit_comment(context, "==================== RET ====================");

    if (ret_node->child_count >= 1)
        gen_expr(ret_node->children[0], identifiers, context);
    else
        emit_two_operands(context, "xorpd", xmm0(), xmm0());

    emit_one_operand(context, "jmp", make_label_fmt("func_end_%zu", context->counters.current_func_id));
}

void gen_expr(node_t* expr_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(expr_node);
    assert(identifiers);
    assert(context);

    switch (expr_node->kind)
    {
        case NODE_VAR:
            emit_two_operands(context, "movsd", xmm0(), make_mem(5, -((int64_t) expr_node->data_t.variable.stack_offset)));
            break;

        case NODE_NUM:  return gen_num(expr_node, context);
        case NODE_OP:   return gen_op_node(expr_node, identifiers, context);
        case NODE_CALL: return gen_call(expr_node, identifiers, context);

        default:
            break;
    }
}

void gen_num(node_t* num_node, instructions_context_t* context)
{
    assert(num_node);
    assert(context);

    char label_name[MAX_LABEL_NAME] = {};
    get_or_add_rodata_double_label(context, num_node->data_t.number,
                                   label_name, sizeof(label_name));

    emit_two_operands(context, "movsd", xmm0(), make_mem_rel(label_name));
}

void gen_call(node_t* call_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(call_node);
    assert(identifiers);
    assert(context);

    node_t* args_node = call_node->children[0];
    int function_id = call_node->data_t.function.id_number;

    if (!strcmp(identifiers[function_id].name, "найдётся") ||
        !strcmp(identifiers[function_id].name, "out"))  { gen_out(call_node, identifiers, context);  return; }
    if (!strcmp(identifiers[function_id].name, "дано") ||
        !strcmp(identifiers[function_id].name, "in"))   { gen_in(call_node, identifiers, context);   return; }
    if (!strcmp(identifiers[function_id].name, "корень") ||
        !strcmp(identifiers[function_id].name, "sqrt")) { gen_sqrt(call_node, identifiers, context); return; }

    emit_comment(context, "==================== CALL \"%s\" ====================",
                 identifiers[function_id].name);

    bool aligned = align_stack_before_call(context);

    if (args_node->child_count >= 1)
    {
        gen_sub_rsp(context, args_node->child_count * sizeof(double));

        for (size_t i = 0; i < args_node->child_count; i++)
        {
            gen_expr(args_node->children[i], identifiers, context);
            emit_two_operands(context, "movsd", make_mem(4, (int64_t) (i * sizeof(double))), xmm0());
            comment_last_instruction(context, "Save function argument %zu", i + 1);
        }
    }

    emit_one_operand(context, "call", make_label_fmt("func_%zu", (size_t) function_id));

    if (args_node->child_count >= 1)
        gen_add_rsp(context, args_node->child_count * sizeof(double));

    unalign_stack_after_call(context, aligned);
}

void gen_out(node_t* out_node, const identifier_t* identifiers, instructions_context_t* context)
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

    emit_comment(context, "==================== OUT ====================");
    gen_expr(args_node->children[0], identifiers, context);

    bool aligned = align_stack_before_call(context);
    emit_one_operand(context, "call", make_label("__out"));
    unalign_stack_after_call(context, aligned);
}

void gen_in(node_t* in_node, const identifier_t* identifiers, instructions_context_t* context)
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

    emit_comment(context, "==================== IN ====================");
    bool aligned = align_stack_before_call(context);
    emit_one_operand(context, "call", make_label("__in"));
    unalign_stack_after_call(context, aligned);
}

void gen_sqrt(node_t* sqrt_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(sqrt_node);
    assert(identifiers);
    assert(context);

    node_t* args_node = sqrt_node->children[0];

    if (args_node->child_count != 1)
    {
        fprintf(stderr, MAKE_BOLD_RED("Error: sqrt() expects only one argument\n"));
        return;
    }

    gen_expr(args_node->children[0], identifiers, context);
    emit_two_operands(context, "sqrtsd", xmm0(), xmm0());
}

void gen_op_node(node_t* op_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(op_node);
    assert(identifiers);
    assert(context);

    if (op_node->data_t.op == LOGIC_OR || op_node->data_t.op == LOGIC_AND)
    {
        gen_logic(op_node, identifiers, context);
        return;
    }

    gen_expr(op_node->children[1], identifiers, context);

    switch (op_node->data_t.op)
    {
        case ADD:           gen_add(op_node, identifiers, context); break;
        case SUB:           gen_sub(op_node, identifiers, context); break;
        case MUL:           gen_mul(op_node, identifiers, context); break;
        case DIV:           gen_div(op_node, identifiers, context); break;

        case ASSIGN:
            emit_two_operands(context, "movsd",
                              make_mem(5, -((int64_t) op_node->children[0]->data_t.variable.stack_offset)),
                              xmm0());
            break;

        case IS_EQUAL:      gen_cmp(op_node, identifiers, context, "je");  break;
        case IS_NOT_EQUAL:  gen_cmp(op_node, identifiers, context, "jne"); break;
        case GREATER_EQUAL: gen_cmp(op_node, identifiers, context, "jae"); break;
        case GREATER:       gen_cmp(op_node, identifiers, context, "ja");  break;
        case LESS_EQUAL:    gen_cmp(op_node, identifiers, context, "jbe"); break;
        case LESS:          gen_cmp(op_node, identifiers, context, "jb");  break;

        default:
            break;
    }
}

void gen_logic(node_t* logic_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(logic_node);
    assert(identifiers);
    assert(context);

    switch (logic_node->data_t.op)
    {
        case LOGIC_OR:
            gen_logic_or(logic_node, identifiers, context);
            return;

        case LOGIC_AND:
            gen_logic_and(logic_node, identifiers, context);
            return;

        default:
            assert(0 && "Expected logical operator");
    }
}

void gen_logic_or(node_t* logic_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(logic_node);
    assert(identifiers);
    assert(context);

    size_t logic_id = ++context->counters.logic_counter;

    gen_expr(logic_node->children[0], identifiers, context);
    emit_two_operands(context, "ucomisd", xmm0(), make_mem_rel("const_false"));
    emit_one_operand(context, "jne", make_label_fmt(".logic_true_%zu", logic_id));

    gen_expr(logic_node->children[1], identifiers, context);
    emit_two_operands(context, "ucomisd", xmm0(), make_mem_rel("const_false"));
    emit_one_operand(context, "jne", make_label_fmt(".logic_true_%zu", logic_id));

    emit_two_operands(context, "movsd", xmm0(), make_mem_rel("const_false"));
    emit_one_operand(context, "jmp", make_label_fmt(".logic_end_%zu", logic_id));

    emit_text_label_fmt(context, ".logic_true_%zu", logic_id);
    emit_two_operands(context, "movsd", xmm0(), make_mem_rel("const_true"));

    emit_text_label_fmt(context, ".logic_end_%zu", logic_id);
}

void gen_logic_and(node_t* logic_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(logic_node);
    assert(identifiers);
    assert(context);

    size_t logic_id = ++context->counters.logic_counter;

    gen_expr(logic_node->children[0], identifiers, context);
    emit_two_operands(context, "ucomisd", xmm0(), make_mem_rel("const_false"));
    emit_one_operand(context, "je", make_label_fmt(".logic_false_%zu", logic_id));

    gen_expr(logic_node->children[1], identifiers, context);
    emit_two_operands(context, "ucomisd", xmm0(), make_mem_rel("const_false"));
    emit_one_operand(context, "je", make_label_fmt(".logic_false_%zu", logic_id));

    emit_two_operands(context, "movsd", xmm0(), make_mem_rel("const_true"));
    emit_one_operand(context, "jmp", make_label_fmt(".logic_end_%zu", logic_id));

    emit_text_label_fmt(context, ".logic_false_%zu", logic_id);
    emit_two_operands(context, "movsd", xmm0(), make_mem_rel("const_false"));

    emit_text_label_fmt(context, ".logic_end_%zu", logic_id);
}

void gen_add(node_t* add_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(add_node);
    assert(identifiers);
    assert(context);

    gen_sub_rsp(context, sizeof(double));
    emit_two_operands(context, "movsd", make_mem(4, 0), xmm0());
    comment_last_instruction(context, "Save temporary value");

    gen_expr(add_node->children[0], identifiers, context);

    emit_two_operands(context, "addsd", xmm0(), make_mem(4, 0));
    gen_add_rsp(context, sizeof(double));
}

void gen_sub(node_t* sub_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(sub_node);
    assert(identifiers);
    assert(context);

    gen_sub_rsp(context, sizeof(double));
    emit_two_operands(context, "movsd", make_mem(4, 0), xmm0());
    comment_last_instruction(context, "Save temporary value");

    gen_expr(sub_node->children[0], identifiers, context);

    emit_two_operands(context, "subsd", xmm0(), make_mem(4, 0));
    gen_add_rsp(context, sizeof(double));
}

void gen_mul(node_t* mul_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(mul_node);
    assert(identifiers);
    assert(context);

    gen_sub_rsp(context, sizeof(double));
    emit_two_operands(context, "movsd", make_mem(4, 0), xmm0());
    comment_last_instruction(context, "Save temporary value");

    gen_expr(mul_node->children[0], identifiers, context);

    emit_two_operands(context, "mulsd", xmm0(), make_mem(4, 0));
    gen_add_rsp(context, sizeof(double));
}

void gen_div(node_t* div_node, const identifier_t* identifiers, instructions_context_t* context)
{
    assert(div_node);
    assert(identifiers);
    assert(context);

    gen_sub_rsp(context, sizeof(double));
    emit_two_operands(context, "movsd", make_mem(4, 0), xmm0());
    comment_last_instruction(context, "Save temporary value");

    gen_expr(div_node->children[0], identifiers, context);

    emit_two_operands(context, "divsd", xmm0(), make_mem(4, 0));
    gen_add_rsp(context, sizeof(double));
}

void gen_cmp(node_t* cmp_node, const identifier_t* identifiers, instructions_context_t* context, const char* jump_word)
{
    assert(cmp_node);
    assert(identifiers);
    assert(context);
    assert(jump_word);

    size_t cmp_id = ++context->counters.cmp_counter;

    gen_sub_rsp(context, sizeof(double));
    emit_two_operands(context, "movsd", make_mem(4, 0), xmm0());
    comment_last_instruction(context, "Save temporary value");

    gen_expr(cmp_node->children[0], identifiers, context);

    emit_two_operands(context, "ucomisd", xmm0(), make_mem(4, 0));
    emit_one_operand(context, jump_word, make_label_fmt(".cmp_true_%zu", cmp_id));

    emit_two_operands(context, "movsd", xmm0(), make_mem_rel("const_false"));
    emit_one_operand(context, "jmp", make_label_fmt(".cmp_end_%zu", cmp_id));

    emit_text_label_fmt(context, ".cmp_true_%zu", cmp_id);
    emit_two_operands(context, "movsd", xmm0(), make_mem_rel("const_true"));

    emit_text_label_fmt(context, ".cmp_end_%zu", cmp_id);

    gen_add_rsp(context, sizeof(double));
}

void gen_add_rsp(instructions_context_t* context, size_t bytes)
{
    assert(context);

    emit_two_operands(context, "add", rsp(), make_imm((int64_t) bytes));
    context->counters.stack_offset -= bytes;
}

void gen_sub_rsp(instructions_context_t* context, size_t bytes)
{
    assert(context);

    emit_two_operands(context, "sub", rsp(), make_imm((int64_t) bytes));
    context->counters.stack_offset += bytes;
}

bool align_stack_before_call(instructions_context_t* context)
{
    assert(context);

    if (context->counters.stack_offset % 16 != 0)
    {
        gen_sub_rsp(context, 8);
        comment_last_instruction(context, "Stack alignment before call");
        return true;
    }

    return false;
}

void unalign_stack_after_call(instructions_context_t* context, bool was_aligned)
{
    assert(context);

    if (was_aligned)
    {
        gen_add_rsp(context, 8);
        comment_last_instruction(context, "Remove stack alignment");
    }
}

static void emit_instruction(instructions_context_t* context, const instruction_t* instruction)
{
    assert(context);
    assert(context->instructions);
    assert(instruction);

    instruction_list_push_back(context->instructions, *instruction);
}

static void emit_rodata_instruction(instructions_context_t* context, const instruction_t* instruction)
{
    assert(context);
    assert(context->rodata_instructions.instructions);
    assert(instruction);

    instruction_list_push_back(&context->rodata_instructions, *instruction);
}

static void emit_text_label(instructions_context_t* context, const char* name)
{
    assert(context);
    assert(context->labels);
    assert(context->instructions);
    assert(name);

    label_t label = {};
    strncpy(label.name, name, sizeof(label.name) - 1);
    label.instruction_index = context->instructions->count;

    label_list_push_back(context->labels, label);
}

static void emit_rodata_label(instructions_context_t* context, const char* name)
{
    assert(context);
    assert(context->rodata_labels.labels);
    assert(name);

    label_t label = {};
    strncpy(label.name, name, sizeof(label.name) - 1);
    label.instruction_index = context->rodata_instructions.count;

    label_list_push_back(&context->rodata_labels, label);
}

static void emit_text_label_fmt(instructions_context_t* context, const char* format, ...)
{
    assert(context);
    assert(format);

    char label_name[MAX_LABEL_NAME] = {};

    va_list args = {};
    va_start(args, format);
    vsnprintf(label_name, sizeof(label_name), format, args);
    va_end(args);

    emit_text_label(context, label_name);
}

static void emit_rodata_double(instructions_context_t* context, const char* label_name, double value)
{
    assert(context);
    assert(label_name);

    emit_rodata_label(context, label_name);

    instruction_t instruction = {};
    strncpy(instruction.mnemonic, "dq", sizeof(instruction.mnemonic) - 1);
    instruction.operands[0] = make_double(value);
    instruction.operand_count = 1;

    emit_rodata_instruction(context, &instruction);
}

static void get_or_add_rodata_double_label(instructions_context_t* context, double value,
                                           char* label_name, size_t label_name_size)
{
    assert(context);
    assert(label_name);
    assert(label_name_size > 0);

    uint64_t value_bits = 0;
    memcpy(&value_bits, &value, sizeof(value_bits));

    for (size_t i = 0; i < context->rodata_instructions.count; i++)
    {
        const instruction_t* instruction = &context->rodata_instructions.instructions[i];
        if (strcmp(instruction->mnemonic, "dq") ||
            instruction->operand_count != 1 ||
            instruction->operands[0].kind != OPERAND_DOUBLE)
            continue;

        uint64_t existing_bits = 0;
        memcpy(&existing_bits, &instruction->operands[0].double_value, sizeof(existing_bits));
        if (existing_bits != value_bits)
            continue;

        for (size_t label_index = 0; label_index < context->rodata_labels.count; label_index++)
        {
            const label_t* label = &context->rodata_labels.labels[label_index];
            if (label->instruction_index == i)
            {
                strncpy(label_name, label->name, label_name_size - 1);
                label_name[label_name_size - 1] = '\0';
                return;
            }
        }

        assert(0 && "Constant instruction must have a label");
    }

    size_t const_id = context->counters.const_counter++;
    snprintf(label_name, label_name_size, "const_%zu", const_id);
    emit_rodata_double(context, label_name, value);
}

static void emit_comment(instructions_context_t* context, const char* format, ...)
{
    assert(context);
    assert(format);

    instruction_t instruction = {};

    va_list args = {};
    va_start(args, format);
    vsnprintf(instruction.comment, sizeof(instruction.comment), format, args);
    va_end(args);

    emit_instruction(context, &instruction);
}

static void comment_last_instruction(instructions_context_t* context, const char* format, ...)
{
    assert(context);
    assert(context->instructions);
    assert(context->instructions->count > 0);
    assert(format);

    instruction_t* instruction = &context->instructions->instructions[context->instructions->count - 1];

    va_list args = {};
    va_start(args, format);
    vsnprintf(instruction->comment, sizeof(instruction->comment), format, args);
    va_end(args);
}

static void emit_no_operands(instructions_context_t* context, const char* mnemonic)
{
    assert(context);
    assert(mnemonic);

    instruction_t instruction = {};
    strncpy(instruction.mnemonic, mnemonic, sizeof(instruction.mnemonic) - 1);

    emit_instruction(context, &instruction);
}

static void emit_one_operand(instructions_context_t* context, const char* mnemonic, operand_t operand)
{
    assert(context);
    assert(mnemonic);

    instruction_t instruction = {};
    strncpy(instruction.mnemonic, mnemonic, sizeof(instruction.mnemonic) - 1);
    instruction.operands[0] = operand;
    instruction.operand_count = 1;

    emit_instruction(context, &instruction);
}

static void emit_two_operands(instructions_context_t* context, const char* mnemonic, operand_t first, operand_t second)
{
    assert(context);
    assert(mnemonic);

    instruction_t instruction = {};
    strncpy(instruction.mnemonic, mnemonic, sizeof(instruction.mnemonic) - 1);
    instruction.operands[0] = first;
    instruction.operands[1] = second;
    instruction.operand_count = 2;

    emit_instruction(context, &instruction);
}

static operand_t make_reg(size_t reg_num, size_t reg_size)
{
    operand_t operand = {};
    operand.kind = OPERAND_REG;
    operand.reg_num = reg_num;
    operand.reg_size = reg_size;

    return operand;
}

static operand_t make_xmm(size_t reg_num)
{
    operand_t operand = {};
    operand.kind = OPERAND_XMM;
    operand.reg_num = reg_num;

    return operand;
}

static operand_t make_mem(size_t base_reg_num, int64_t displacement)
{
    operand_t operand = {};
    operand.kind = OPERAND_MEM;
    operand.reg_num = base_reg_num;
    operand.reg_size = 8;
    operand.displacement = displacement;

    return operand;
}

static operand_t make_mem_rel(const char* label_name)
{
    assert(label_name);

    operand_t operand = {};
    operand.kind = OPERAND_MEM_REL;
    strncpy(operand.label_name, label_name, sizeof(operand.label_name) - 1);

    return operand;
}

static operand_t make_imm(int64_t value)
{
    operand_t operand = {};
    operand.kind = OPERAND_IMM;
    operand.imm_value = value;

    return operand;
}

static operand_t make_double(double value)
{
    operand_t operand = {};
    operand.kind = OPERAND_DOUBLE;
    operand.double_value = value;

    return operand;
}

static operand_t make_label(const char* label_name)
{
    assert(label_name);

    operand_t operand = {};
    operand.kind = OPERAND_LABEL;
    strncpy(operand.label_name, label_name, sizeof(operand.label_name) - 1);

    return operand;
}

static operand_t make_label_fmt(const char* format, ...)
{
    assert(format);

    operand_t operand = {};
    operand.kind = OPERAND_LABEL;

    va_list args = {};
    va_start(args, format);
    vsnprintf(operand.label_name, sizeof(operand.label_name), format, args);
    va_end(args);

    return operand;
}

static void append_rodata(instructions_context_t* context)
{
    assert(context);
    assert(context->instructions);
    assert(context->labels);

    size_t text_instruction_count = context->instructions->count;

    for (size_t i = 0; i < context->rodata_labels.count; i++)
    {
        label_t label = context->rodata_labels.labels[i];
        label.instruction_index += text_instruction_count;
        label_list_push_back(context->labels, label);
    }

    for (size_t i = 0; i < context->rodata_instructions.count; i++)
        instruction_list_push_back(context->instructions, context->rodata_instructions.instructions[i]);
}

static size_t align_up_16(size_t number)
{
    return (number + 15) / 16 * 16;
}
