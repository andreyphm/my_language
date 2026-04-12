#ifndef MACROS_H
#define MACROS_H

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
