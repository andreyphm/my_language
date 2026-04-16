#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "front_end.h"
#include "tree.h"
#include "tokenization.h"
#include "parser.h"
#include "macros.h"
#include "font.h"

#define TOKEN_IS_OP                 (*token)->type == OP
#define TOKEN_IS_SPEC               (*token)->type == SPEC
#define TOKEN_IS_NUM                (*token)->type == NUM
#define TOKEN_IS_ID                 (*token)->type == ID
#define TOKEN_IS_KEYWORD            (*token)->type == KEYWORD
#define TOKEN_SPEC_SYMBOL           (*token)->data_t.spec_symbol
#define TOKEN_OP_CODE               (*token)->data_t.op
#define TOKEN_KEYWORD_CODE          (*token)->data_t.keyword
#define TOKEN_NUM_VALUE             (*token)->data_t.number
#define TOKEN_ID_NUMBER             (*token)->data_t.id_number
#define NEXT_TOKEN_IS_SPEC          ((*token)->next)->type == SPEC
#define NEXT_TOKEN_SPEC_SYMBOL      ((*token)->next)->data_t.spec_symbol
#define NEXT_TOKEN_IS_OP            ((*token)->next)->type == OP
#define NEXT_TOKEN_OP_CODE          ((*token)->next)->data_t.op

error_code tokens_to_tree(list_t* list, node_t** node_ptr)
{
    token_t* current = list->head;
    *node_ptr = get_prog(&current);

    list_destroy(list);
    if (!*node_ptr) return TREE_NULLPTR;

    printf(MAKE_BOLD_GREEN("Parsed successfully\n"));
    return NO_ERROR;
}

node_t* get_prog(token_t** token)
{
    node_t* prog = create_prog_node();
    if (!prog) return nullptr;

    while (TOKEN_IS_KEYWORD && TOKEN_KEYWORD_CODE == FUNC)
    {
        node_t* func = get_func(token);
        if (!func) return destroy_and_null(prog);

        node_add_child(prog, func);
    }

    if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == '$'))
        return destroy_and_null(prog);

    return prog;
}

node_t* get_func(token_t** token)
{
    if (!(TOKEN_IS_KEYWORD && TOKEN_KEYWORD_CODE == FUNC)) return nullptr;
    *token = (*token)->next;

    if (!(TOKEN_IS_ID)) return nullptr;
    int func_id_num = TOKEN_ID_NUMBER;
    *token = (*token)->next;

    if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == '(')) return nullptr;
    *token = (*token)->next;

    node_t* params = get_params(token);
    if (!params) return nullptr;

    if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == ')')) return destroy_and_null(params);
    *token = (*token)->next;

    node_t* func_body = get_block(token);
    if (!func_body) return destroy_and_null(params);

    return create_func_node(func_id_num, params, func_body);
}

node_t* get_params(token_t** token)
{
    node_t* params = create_args_node();

    if (TOKEN_IS_ID)
    {
        node_t* first_arg = get_id(token);
        if (!first_arg) return destroy_and_null(params);

        node_add_child(params, first_arg);
    }
    while (TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == ',')
    {
        *token = (*token)->next;
        node_t* arg = get_id(token);
        if (!arg) return destroy_and_null(params);

        node_add_child(params, arg);
    }

    return params;
}

node_t* get_block(token_t** token)
{
    if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == '{')) return nullptr;
    *token = (*token)->next;

    node_t* body = create_body_node();
    if (!body) return nullptr;

    while ((TOKEN_IS_KEYWORD && !(TOKEN_KEYWORD_CODE == FUNC) && !(TOKEN_KEYWORD_CODE == ELSE)) 
            || token_is_start_of_expr(token))
    {
        node_t* op = get_op(token);
        if (!op) return destroy_and_null(body);
        
        node_add_child(body, op);
    }
    
    if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == '}')) 
    {
        destroy_node(body);
        return nullptr;
    }
    *token = (*token)->next;

    return body;
}

node_t* get_op(token_t** token)
{
    if (TOKEN_IS_KEYWORD)
    {
        switch(TOKEN_KEYWORD_CODE)
        {
            case IF:        return get_if(token);
            case WHILE:     return get_while(token);
            case VAR_DECL:  return get_var_declare(token);
            case RET:       return get_ret(token);
            case BREAK:     return get_break(token);
            default:        return nullptr;
        }
    }
    
    node_t* value = get_e(token);
    if (!value) return nullptr;

    if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == ';')) 
        return destroy_and_null(value);
    *token = (*token)->next;

    return value;
}

node_t* get_if(token_t** token)
{
    if (!(TOKEN_IS_KEYWORD && TOKEN_KEYWORD_CODE == IF)) return nullptr;
    *token = (*token)->next;

    if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == '(')) return nullptr;
    *token = (*token)->next;

    node_t* cond = get_e(token);
    if (!cond) return nullptr;

    if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == ')')) return destroy_and_null(cond);
    *token = (*token)->next;

    node_t* then_body = get_block(token);
    if (!then_body) return destroy_and_null(cond);

    node_t* else_body = nullptr;
    if (TOKEN_IS_KEYWORD && TOKEN_KEYWORD_CODE == ELSE)
    {
        *token = (*token)->next;
        else_body = get_block(token);
        if (!else_body)
        {
            destroy_node(cond);
            destroy_node(then_body);
            return nullptr;
        }
        else_body->kind = NODE_ELSE;
    }    

    return create_if_node(cond, then_body, else_body);
}

node_t* get_while(token_t** token)
{
    if (!(TOKEN_IS_KEYWORD && TOKEN_KEYWORD_CODE == WHILE)) return nullptr;
    *token = (*token)->next;

    if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == '(')) return nullptr;
    *token = (*token)->next;

    node_t* cond = get_e(token);
    if (!cond) return nullptr;

    if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == ')')) return destroy_and_null(cond);
    *token = (*token)->next;

    node_t* then_body = get_block(token);
    if (!then_body) return destroy_and_null(cond);

    return create_while_node(cond, then_body);
}

node_t* get_var_declare(token_t** token)
{
    if (!(TOKEN_IS_KEYWORD && TOKEN_KEYWORD_CODE == VAR_DECL)) return nullptr;
    *token = (*token)->next;

    node_t* var_name = get_id(token);
    if (!var_name) return nullptr;

    if (TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == ';')
    {
        *token = (*token)->next;
        return create_var_decl_node(var_name, nullptr);
    }

    if (TOKEN_IS_OP && TOKEN_OP_CODE == ASSIGN)
    {
        *token = (*token)->next;
        node_t* var_value = get_e(token);
        if (!var_value) return destroy_and_null(var_name);

        if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == ';'))
        {
            destroy_node(var_name);
            destroy_node(var_value);
            return nullptr;
        } 
        *token = (*token)->next;
        return create_var_decl_node(var_name, var_value);
    }

    destroy_node(var_name);
    return nullptr;
}

node_t* get_ret(token_t** token)
{
    if (!(TOKEN_IS_KEYWORD && TOKEN_KEYWORD_CODE == RET)) return nullptr;
    *token = (*token)->next;

    if (TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == ';')
    {
        *token = (*token)->next;
        return create_ret_node(nullptr);
    }

    node_t* value = get_e(token);
    if (!value) return nullptr;

    if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == ';'))
        return destroy_and_null(value);
    *token = (*token)->next;

    return create_ret_node(value);
}

node_t* get_break(token_t** token)
{
    if (!(TOKEN_IS_KEYWORD && TOKEN_KEYWORD_CODE == BREAK)) return nullptr;
    *token = (*token)->next;

    if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == ';')) return nullptr;
    *token = (*token)->next; 

    return create_break_node();
}

node_t* get_e(token_t** token)
{
    if (TOKEN_IS_ID && NEXT_TOKEN_IS_OP && NEXT_TOKEN_OP_CODE == ASSIGN)
    { 
        node_t* var_name = get_id(token);
        if (!var_name) return nullptr;

        *token = (*token)->next;

        node_t* var_value = get_e(token);
        if (!var_value) return destroy_and_null(var_name);

        return create_op_node(ASSIGN, var_name, var_value);
    }

    return get_or(token);
}

node_t* get_or(token_t** token)
{
    node_t* value = get_and(token);
    if (!value) return nullptr;

    while (TOKEN_IS_OP && TOKEN_OP_CODE == LOGIC_OR)
    {
        *token = (*token)->next;

        node_t* value_2 = get_and(token);
        if (!value_2) return destroy_and_null(value);

        value = create_op_node(LOGIC_OR, value, value_2);
    }

    return value;
}

node_t* get_and(token_t** token)
{
    node_t* value = get_bit_or(token);
    if (!value) return nullptr;

    while (TOKEN_IS_OP && TOKEN_OP_CODE == LOGIC_AND)
    {
        *token = (*token)->next;

        node_t* value_2 = get_bit_or(token);
        if (!value_2) return destroy_and_null(value);

        value = create_op_node(LOGIC_AND, value, value_2);
    }

    return value;
}

node_t* get_bit_or(token_t** token)
{
    node_t* value = get_bit_xor(token);
    if (!value) return nullptr;

    while (TOKEN_IS_OP && TOKEN_OP_CODE == BIT_OR)
    {
        *token = (*token)->next;

        node_t* value_2 = get_bit_xor(token);
        if (!value_2) return destroy_and_null(value);

        value = create_op_node(BIT_OR, value, value_2);
    }

    return value;
}

node_t* get_bit_xor(token_t** token)
{
    node_t* value = get_bit_and(token);
    if (!value) return nullptr;

    while (TOKEN_IS_OP && TOKEN_OP_CODE == BIT_XOR)
    {
        *token = (*token)->next;

        node_t* value_2 = get_bit_and(token);
        if (!value_2) return destroy_and_null(value);

        value = create_op_node(BIT_XOR, value, value_2);
    }

    return value;
}

node_t* get_bit_and(token_t** token)
{
    node_t* value = get_equal_or_not(token);
    if (!value) return nullptr;

    while (TOKEN_IS_OP && TOKEN_OP_CODE == BIT_AND)
    {
        *token = (*token)->next;

        node_t* value_2 = get_equal_or_not(token);
        if (!value_2) return destroy_and_null(value);

        value = create_op_node(BIT_AND, value, value_2);
    }

    return value;
}

node_t* get_equal_or_not(token_t** token)
{
    node_t* value = get_less_greater(token);
    if (!value) return nullptr;

    while (TOKEN_IS_OP && (TOKEN_OP_CODE == IS_EQUAL || TOKEN_OP_CODE == IS_NOT_EQUAL))
    {
        operator_code op = TOKEN_OP_CODE;
        *token = (*token)->next;

        node_t* value_2 = get_less_greater(token);
        if (!value_2) return destroy_and_null(value);

        if (op == IS_EQUAL)
            value = create_op_node(IS_EQUAL, value, value_2);
        else
            value = create_op_node(IS_NOT_EQUAL, value, value_2);
    }

    return value;
}

node_t* get_less_greater(token_t** token)
{
    node_t* value = get_shift(token);
    if (!value) return nullptr;

    while (TOKEN_IS_OP && (TOKEN_OP_CODE == GREATER_EQUAL || TOKEN_OP_CODE == GREATER
                           || TOKEN_OP_CODE == LESS_EQUAL || TOKEN_OP_CODE == LESS))
    {
        operator_code op = TOKEN_OP_CODE;
        *token = (*token)->next;

        node_t* value_2 = get_shift(token);
        if (!value_2) return destroy_and_null(value);

        switch(op)
        {
            case GREATER_EQUAL:
            case GREATER:
            case LESS_EQUAL:
            case LESS:
                value = create_op_node(op, value, value_2);
                break;
            default:
                destroy_node(value);
                destroy_node(value_2);
                return nullptr;
        }
    }

    return value;
}

node_t* get_shift(token_t** token)
{
    node_t* value = get_add_sub(token);
    if (!value) return nullptr;

    while (TOKEN_IS_OP && (TOKEN_OP_CODE == SHL || TOKEN_OP_CODE == SHR))
    {
        operator_code op = TOKEN_OP_CODE;
        *token = (*token)->next;

        node_t* value_2 = get_add_sub(token);
        if (!value_2) return destroy_and_null(value);

        value = create_op_node(op, value, value_2);
    }

    return value;
}

node_t* get_add_sub(token_t** token)
{
    node_t* value = get_mul_div(token);
    if (!value) return nullptr;

    while (TOKEN_IS_OP && (TOKEN_OP_CODE == ADD || TOKEN_OP_CODE == SUB))
    {
        operator_code op = TOKEN_OP_CODE;
        *token = (*token)->next;

        node_t* value_2 = get_mul_div(token);
        if (!value_2) return destroy_and_null(value);

        value = create_op_node(op, value, value_2);
    }

    return value;
}

node_t* get_mul_div(token_t** token)
{
    node_t* value = get_p(token);
    if (!value) return nullptr;

    while (TOKEN_IS_OP && (TOKEN_OP_CODE == MUL || TOKEN_OP_CODE == DIV))
    {
        operator_code op = TOKEN_OP_CODE;
        *token = (*token)->next;

        node_t* value_2 = get_p(token);
        if (!value_2) return destroy_and_null(value);

        value = create_op_node(op, value, value_2);
    }

    return value;
}

node_t* get_p(token_t** token)
{
    if (TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == '(')
    {
        *token = (*token)->next;
        node_t* value = get_e(token);
        if (!value) return nullptr;

        if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == ')')) return destroy_and_null(value);
        *token = (*token)->next;

        return value;
    }

    else if (TOKEN_IS_NUM)
        return get_n(token);

    else if (TOKEN_IS_ID && NEXT_TOKEN_IS_SPEC && NEXT_TOKEN_SPEC_SYMBOL == '(')
        return get_call(token);

    return get_id(token);
}

node_t* get_call(token_t** token)
{
    if (!(TOKEN_IS_ID)) return nullptr;
    int func_id_num = TOKEN_ID_NUMBER;
    *token = (*token)->next;

    if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == '(')) return nullptr;
    *token = (*token)->next;

    node_t* args = get_args(token);

    if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == ')')) return destroy_and_null(args);
    *token = (*token)->next;

    return create_call_node(func_id_num, args);
}

node_t* get_args(token_t** token)
{
    node_t* args = create_args_node();

    node_t* first_arg = get_e(token);
    if (!first_arg) return args;
    
    node_add_child(args, first_arg);

    while (TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == ',')
    {
        *token = (*token)->next;

        node_t* arg = get_e(token);
        if (!arg) return destroy_and_null(args);
        node_add_child(args, arg);
    }

    return args;
}

node_t* get_n(token_t** token)
{
    node_t* value = create_num_node(TOKEN_NUM_VALUE);
    *token = (*token)->next;

    return value;
}

node_t* get_id(token_t** token)
{
    if (!(TOKEN_IS_ID)) return nullptr;

    node_t* value = create_var_node(TOKEN_ID_NUMBER);
    *token = (*token)->next;

    return value;
}

bool token_is_start_of_expr(token_t* const* const token)
{
    return ((TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == '(') || TOKEN_IS_NUM || TOKEN_IS_ID);
}
