#ifndef MACROS_H
#define MACROS_H

#define ADD_(left_node, right_node) create_node(OP,  (data_union){.op = ADD}, left_node, right_node)
#define SUB_(left_node, right_node) create_node(OP,  (data_union){.op = SUB}, left_node, right_node)
#define MUL_(left_node, right_node) create_node(OP,  (data_union){.op = MUL}, left_node, right_node)
#define DIV_(left_node, right_node) create_node(OP,  (data_union){.op = DIV}, left_node, right_node)
#define POW_(left_node, right_node) create_node(OP,  (data_union){.op = POW}, left_node, right_node)
#define EXP_(left_node)             create_node(OP,  (data_union){.op = EXP}, left_node, nullptr)
#define LN_(left_node)              create_node(OP,  (data_union){.op = LN},  left_node, nullptr)
#define SIN_(left_node)             create_node(OP,  (data_union){.op = SIN}, left_node, nullptr)
#define COS_(left_node)             create_node(OP,  (data_union){.op = COS}, left_node, nullptr)
#define NUM_(value)                 create_node(NUM, (data_union){.number = (value)}, nullptr, nullptr)
#define VAR_(value)                 create_node(VAR, (data_union){.var_number = (value)}, nullptr, nullptr)

#define CR                  copy_node(node->right)
#define CL                  copy_node(node->left)

#define NODE_TYPE           node->value->type
#define NODE_NUMBER         node->value->data_t.number
#define NODE_OPERATION      node->value->data_t.op
#define PARENT_OPERATION    parent->value->data_t.op
#define NODE_VAR_NUMBER     node->value->data_t.var_number

#define RIGHT_IS_NUMBER     (node->right->value->type == NUM)
#define LEFT_IS_NUMBER      (node->left->value->type == NUM)
#define RIGHT_VALUE         node->right->value->data_t.number
#define LEFT_VALUE          node->left->value->data_t.number

#endif // MACROS_H
