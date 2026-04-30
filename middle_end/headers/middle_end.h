#ifndef MIDDLE_END_H
#define MIDDLE_END_H

#include "tree.h"

const int NUMBER_CLOSE_TO_ZERO = 10e-12;

#define FIRST_CHILD(node)           node->children[0]
#define SECOND_CHILD(node)          node->children[1]
#define IS_NUM_NODE_(node)          (node->kind == NODE_NUM)
#define NUM_VAL_(node)              node->data_t.number
#define OP_VAL_(node)               node->data_t.op
#define NUM_NODE_(value)            create_node(NODE_NUM, (data_union){.number = (value)})
#define MUL_NODE_()                 create_node(NODE_OP,  (data_union){.op = MUL})

void middle_end_run(node_t* tree);

node_t* simplify_node(node_t* node, bool* simplifications_ptr);
bool is_close_to_zero (double number_being_checked);

#endif // MIDDLE_END_H
