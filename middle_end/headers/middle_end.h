#ifndef SIMPLIFIER_H
#define SIMPLIFIER_H

#include "front_end.h"

#define FIRST_CHILD(node)           node->children[0]
#define SECOND_CHILD(node)          node->children[1]
#define IS_NUM_NODE_(node)          (node->kind == NODE_NUM)
#define NUM_VAL_(node)              node->data_t.number
#define OP_VAL_(node)               node->data_t.op
#define NUM_NODE_(value)            create_node(NODE_NUM, (data_union){.number = (value)})
#define MUL_NODE_()                 create_node(NODE_OP,  (data_union){.op = MUL})

void middle_end_run(node_t* tree);

node_t* simplify_node(node_t* node, bool* simplifications_ptr);

#endif // SIMPLIFIER_H
