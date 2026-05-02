#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "middle_end.h"
#include "tree.h"

node_t* simplify_node(node_t* node, bool* simplifications)
{
    if (!node) return nullptr;
    if (!node->children) return node;
    // if (!FIRST_CHILD(node) || !SECOND_CHILD(node)) return node;

    switch(node->kind)
    {
        case NODE_OP:
            if (IS_NUM_NODE_(FIRST_CHILD(node)) && IS_NUM_NODE_(SECOND_CHILD(node)))
            {
                *simplifications = true;
                double left_number  = NUM_VAL_(FIRST_CHILD(node));
                double right_number = NUM_VAL_(SECOND_CHILD(node));
                operator_code op_code = OP_VAL_(node);
                destroy_node(node);

                switch(op_code)
                {
                    case ADD: return NUM_NODE_(left_number + right_number);
                    case SUB: return NUM_NODE_(left_number - right_number);
                    case MUL: return NUM_NODE_(left_number * right_number);
                    case DIV: return NUM_NODE_(left_number / right_number);
                    default:  break;
                }
            }
            switch(OP_VAL_(node))
            {
                case MUL:
                    if ((IS_NUM_NODE_(FIRST_CHILD(node)) && is_close_to_zero(NUM_VAL_(FIRST_CHILD(node)))) || 
                        (IS_NUM_NODE_(SECOND_CHILD(node)) && is_close_to_zero(NUM_VAL_(SECOND_CHILD(node)))))
                    {
                        *simplifications = true;
                        destroy_node(node);
                        return NUM_NODE_(0);
                    }

                    else if (IS_NUM_NODE_(SECOND_CHILD(node)) && is_close_to_zero(NUM_VAL_(SECOND_CHILD(node)) - 1))
                    {
                        *simplifications = true;
                        node_t* new_node = copy_node(FIRST_CHILD(node));
                        destroy_node(node);
                        return new_node;
                    }

                    else if (IS_NUM_NODE_(FIRST_CHILD(node)) && is_close_to_zero(NUM_VAL_(FIRST_CHILD(node)) - 1))
                    {
                        *simplifications = true;
                        node_t* new_node = copy_node(SECOND_CHILD(node));
                        destroy_node(node);
                        return new_node;
                    }
                    break;

                case ADD:
                    if ((IS_NUM_NODE_(SECOND_CHILD(node)) && is_close_to_zero(NUM_VAL_(SECOND_CHILD(node)))))
                    {
                        *simplifications = true;
                        node_t* new_node = copy_node(FIRST_CHILD(node));
                        destroy_node(node);
                        return new_node;
                    }

                    else if (IS_NUM_NODE_(FIRST_CHILD(node)) && is_close_to_zero(NUM_VAL_(FIRST_CHILD(node))))
                    {
                        *simplifications = true;
                        node_t* right_node = copy_node(SECOND_CHILD(node));
                        destroy_node(node);
                        return right_node;
                    }
                    break;

                case DIV:
                    if (IS_NUM_NODE_(FIRST_CHILD(node)) && is_close_to_zero(NUM_VAL_(FIRST_CHILD(node))))
                    {
                        *simplifications = true;
                        destroy_node(node);
                        return NUM_NODE_(0);
                    }
                    break;
                case SUB:
                    if ((IS_NUM_NODE_(SECOND_CHILD(node)) && is_close_to_zero(NUM_VAL_(SECOND_CHILD(node)))))
                    {
                        *simplifications = true;
                        node_t* new_node = copy_node(FIRST_CHILD(node));
                        destroy_node(node);
                        return new_node;
                    }

                    else if (IS_NUM_NODE_(FIRST_CHILD(node)) && is_close_to_zero(NUM_VAL_(FIRST_CHILD(node))))
                    {
                        *simplifications = true;
                        node_t* right_node = copy_node(FIRST_CHILD(node));
                        destroy_node(node);

                        node_t* new_node = MUL_NODE_();
                        FIRST_CHILD(new_node) = NUM_NODE_(-1);
                        SECOND_CHILD(new_node) = right_node;
                    }
                default:
                    break;
            }
        default:
            break;
    }
    FIRST_CHILD(node)  = simplify_node(FIRST_CHILD(node), simplifications);
    SECOND_CHILD(node) = simplify_node(SECOND_CHILD(node), simplifications);

    return node;
}

node_t* copy_node(node_t* node)
{
    if (!node) return nullptr;

    node_t* new_node = (node_t*) calloc(1, sizeof(node_t));
    assert(new_node);

    new_node->kind = node->kind;
    new_node->unique_id = node->unique_id;

    switch(new_node->kind)
    {
        case NODE_OP:
            new_node->data_t.op = node->data_t.op;
            break;

        case NODE_VAR:
            new_node->data_t.id_number = node->data_t.id_number;
            break;

        case NODE_NUM:
            new_node->data_t.number = node->data_t.number;
            break;

        default:
            break;
    }

    if (FIRST_CHILD(node))
        FIRST_CHILD(new_node) = copy_node(FIRST_CHILD(node));
    else
        FIRST_CHILD(new_node) = nullptr;

    if (SECOND_CHILD(node))
        SECOND_CHILD(new_node) = copy_node(SECOND_CHILD(node));
    else
        SECOND_CHILD(new_node) = nullptr;

    return new_node;
}

bool is_close_to_zero (double number_being_checked)
{
    return (fabs(number_being_checked) < NUMBER_CLOSE_TO_ZERO);
}
