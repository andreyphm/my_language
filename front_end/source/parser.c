#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "front_end.h"
#include "node_functions.h"
#include "tokenization.h"
#include "parser.h"
#include "macros.h"
#include "font.h"

#define TOKEN_IS_OP                 (*token)->type == OP
#define TOKEN_IS_SPEC               (*token)->type == SPEC
#define TOKEN_IS_NUM                (*token)->type == NUM
#define TOKEN_IS_VAR                (*token)->type == VAR
#define TOKEN_SPEC_SYMBOL           (*token)->data_t.spec_symbol
#define TOKEN_OP_CODE               (*token)->data_t.op
#define TOKEN_NUM_VALUE             (*token)->data_t.number
#define TOKEN_VAR_NUMBER            (*token)->data_t.var_number

error_code tokens_to_tree(list_t* list, node_t** node_ptr)
{
    // if (*node_ptr) destroy_node(*node_ptr);
    token_t* current = list->head;
    *node_ptr = GetG(&current);

    list_destroy(list);
    // *variables_ptr = variables;
    if (!*node_ptr) return TREE_NULLPTR;

    printf(MAKE_BOLD_GREEN("Successfully\n"));
    return NO_ERROR;
}

node_t* GetG(token_t** token)
{
    node_t* value = GetE(token);
    if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == '$'))
    {
        destroy_node(value);
        return nullptr;
    }
    return value;
}

node_t* GetE(token_t** token)
{
    node_t* value = GetT(token);
    if (!value) return nullptr;

    while (TOKEN_IS_OP && (TOKEN_OP_CODE == ADD || TOKEN_OP_CODE == SUB))
    {
        operator_code op = TOKEN_OP_CODE;
        *token = (*token)->next;

        node_t* value_2 = GetT(token);
        if (!value_2)
        {
            destroy_node(value);
            return nullptr;
        }

        if (op == ADD)
            value = ADD_(value, value_2);
        else
            value = SUB_(value, value_2);
    }

    return value;
}

node_t* GetT(token_t** token)
{
    node_t* value = GetS(token);
    if (!value) return nullptr;

    while (TOKEN_IS_OP && (TOKEN_OP_CODE == MUL || TOKEN_OP_CODE == DIV))
    {
        operator_code op = TOKEN_OP_CODE;
        *token = (*token)->next;

        node_t* value_2 = GetS(token);
        if (!value_2)
        {
            destroy_node(value);
            return nullptr;
        }

        if (op == MUL)
            value = MUL_(value, value_2);
        else
            value = DIV_(value, value_2);
    }

    return value;
}

node_t* GetS(token_t** token)
{
    node_t* value = GetP(token);
    if (!value) return nullptr;

    while (TOKEN_IS_OP && TOKEN_OP_CODE == POW)
    {
        *token = (*token)->next;

        node_t* value_2 = GetP(token);
        if (!value_2)
        {
            destroy_node(value);
            return nullptr;
        }

        value = POW_(value, value_2);
    }

    return value;
}

node_t* GetP(token_t** token)
{
    if (TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == '(')
    {
        *token = (*token)->next;
        node_t* value = GetE(token);
        if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == ')'))
        {
            destroy_node(value);
            return nullptr;
        }
        *token = (*token)->next;
        return value;
    }

    else if (TOKEN_IS_NUM)
        return GetN(token);

    else if (TOKEN_IS_VAR)
        return GetV(token);

    else if (TOKEN_IS_OP && TOKEN_OP_CODE <= LAST_FUNC_NUM && TOKEN_OP_CODE >= FIRST_FUNC_NUM)
        return GetF(token);

    else return nullptr;
}

node_t* GetN(token_t** token)
{
    node_t* value = NUM_(TOKEN_NUM_VALUE);
    *token = (*token)->next;

    return value;
}

node_t* GetV(token_t** token)
{
    node_t* value = VAR_(TOKEN_VAR_NUMBER);
    *token = (*token)->next;

    return value;
}

node_t* GetF(token_t** token)
{
    operator_code code = TOKEN_OP_CODE;
    *token = (*token)->next;

    if (TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == '(')
    {
        *token = (*token)->next;
        node_t* value = GetE(token);
        if (!(TOKEN_IS_SPEC && TOKEN_SPEC_SYMBOL == ')'))
        {
            destroy_node(value);
            return nullptr;
        }

        *token = (*token)->next;

        switch(code)
        {
            case LN:  return LN_(value);
            case COS: return COS_(value);
            case SIN: return SIN_(value);
            case EXP: return EXP_(value);
            case ADD:
            case SUB:
            case MUL:
            case DIV:
            case POW:
            case ASSIGN:
            default: return nullptr;
        }
    }

    return nullptr;
}
