#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <math.h>

#include "node_functions.h"
#include "front_end.h"
#include "font.h"
#include "macros.h"

node_t* create_node(node_kind kind, data_union data)
{
    node_t* node = (node_t*) calloc(1, sizeof(node_t));
    assert(node);

    node->kind = kind;
    node->data_t = data;
    node->children = nullptr;
    node->child_count = 0;
    node->child_capacity = 0;

    return node;
}

void node_reserve(node_t* node, size_t new_capacity)
{
    assert(node);

    if (new_capacity <= node->child_capacity) return;

    node_t** new_children = nullptr;

    if (!node->children)
        new_children = (node_t**) calloc(new_capacity, sizeof(node_t*));
    else
        new_children = (node_t**) realloc(node->children, new_capacity * sizeof(node_t*));

    node->children = new_children;
    node->child_capacity = new_capacity;
}

void node_add_child(node_t* parent, node_t* child)
{
    assert(parent);
    if (!child) return;

    if (parent->child_count >= parent->child_capacity)
    {
        size_t new_capacity = (parent->child_capacity == 0) ? FIRST_CHILDREN_NUMBER : parent->child_capacity * 2;
        node_reserve(parent, new_capacity);
    }

    parent->children[parent->child_count] = child;
    parent->child_count++;

    return;
}

void destroy_node(node_t* node)
{
    if (!node) return;

    for (size_t i = 0; i < node->child_count; i++)
        destroy_node(node->children[i]);

    free(node->children);
    free(node);
}

node_t* create_prog_node()
{
    return create_node(NODE_PROG, (data_union){});
}

node_t* create_num_node(double value)
{
    node_t* node = create_node(NODE_NUM, (data_union){.number = value});
    return node;
}

node_t* create_var_node(int var_id)
{
    node_t* node = create_node(NODE_VAR, (data_union){.id_number = var_id});
    return node;
}

node_t* create_op_node(operator_code op, node_t* left, node_t* right)
{
    node_t* node = create_node(NODE_OP, (data_union){.op = op});
    node_add_child(node, left);
    node_add_child(node, right);

    return node;
}

node_t* create_body_node()
{
    return create_node(NODE_BODY, (data_union){});
}

node_t* create_args_node()
{
    return create_node(NODE_ARGS, (data_union){});
}

node_t* create_func_node(int func_id_num, node_t* args, node_t* body)
{
    node_t* node = create_node(NODE_FUNC, (data_union){.id_number = func_id_num});
    node_add_child(node, args);
    node_add_child(node, body);

    return node;
}

node_t* create_call_node(int func_id_num, node_t* args)
{
    node_t* node = create_node(NODE_CALL, (data_union){.id_number = func_id_num});
    node_add_child(node, args);

    return node;
}

node_t* create_if_node(node_t* condition, node_t* then_body, node_t* else_body)
{
    node_t* node = create_node(NODE_IF, (data_union){});
    node_add_child(node, condition);
    node_add_child(node, then_body);
    node_add_child(node, else_body);

    return node;
}

node_t* create_else_node()
{
    return create_node(NODE_ELSE, (data_union){});
}

node_t* create_while_node(node_t* condition, node_t* body)
{
    node_t* node = create_node(NODE_WHILE, (data_union){});
    node_add_child(node, condition);
    node_add_child(node, body);

    return node;
}

node_t* create_var_decl_node(node_t* var, node_t* init)
{
    node_t* node = create_node(NODE_VAR_DECL, (data_union){});
    node_add_child(node, var);
    node_add_child(node, init);

    return node;
}

node_t* create_ret_node(node_t* expr)
{
    node_t* node = create_node(NODE_RET, (data_union){});
    node_add_child(node, expr);

    return node;
}

node_t* create_break_node()
{
    return create_node(NODE_BREAK, (data_union){});
}

node_t* create_cond_node()
{
    return create_node(NODE_COND, (data_union){});
}

node_t* destroy_and_null(node_t* node)
{
    destroy_node(node);
    return nullptr;
}

bool is_close_to_zero (double number_being_checked)
{
    return (fabs(number_being_checked) < NUMBER_CLOSE_TO_ZERO);
}
