#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <math.h>

#include "node_functions.h"
#include "front_end.h"
#include "font.h"
#include "macros.h"

node_t* copy_node(node_t* node)
{
    if (!node) return nullptr;

    node_t* new_node = (node_t*) calloc(1, sizeof(node_t));
    assert(new_node);

    new_node->value = (node_value*) calloc(1, sizeof(node_value));
    assert(new_node->value);

    new_node->value->type = NODE_TYPE;

    switch(new_node->value->type)
    {
        case OP:
            new_node->value->data_t.op = node->value->data_t.op;
            break;

        case VAR:
            new_node->value->data_t.var_number = node->value->data_t.var_number;
            break;

        case NUM:
            new_node->value->data_t.number = node->value->data_t.number;
            break;

        case SPEC:
        default:
            break;
    }

    if (node->left)
        new_node->left = CL;
    else
        new_node->left = nullptr;

    if (node->right)
        new_node->right = CR;
    else
        new_node->right = nullptr;

    return new_node;
}

node_t* create_node(const type_data type, data_union data, node_t* left, node_t* right)
{
    node_t* node = (node_t*) calloc(1, sizeof(node_t));
    assert(node);

    node->value = (node_value*) calloc(1, sizeof(node_value));
    assert(node->value);

    NODE_TYPE = type;
    switch(type)
    {
        case OP:
            node->value->data_t.op = data.op;
            break;

        case VAR:
            node->value->data_t.var_number = data.var_number;
            break;

        case NUM:
            node->value->data_t.number = data.number;
            break;

        case SPEC:
        default:
            break;
    }

    node->left = left;
    node->right = right;

    return node;
}

void destroy_node(node_t* node)
{
    if (!node) return;

    destroy_node(node->left);
    destroy_node(node->right);

    if (node->value)
        free(node->value);

    free(node);
}

bool is_close_to_zero (double number_being_checked)
{
    return (fabs(number_being_checked) < NUMBER_CLOSE_TO_ZERO);
}
