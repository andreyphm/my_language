#include "front_end.h"

node_t* create_node(node_kind kind, data_union data);
void node_reserve(node_t* node, size_t new_capacity);
void node_add_child(node_t* parent, node_t* child);
node_t* create_prog_node();
node_t* create_num_node(double value);
node_t* create_var_node(int var_id);
node_t* create_op_node(operator_code op, node_t* left, node_t* right);
node_t* create_body_node();
node_t* create_args_node();
node_t* create_func_node(int func_id_num, node_t* args, node_t* body);
node_t* create_call_node(int func_id_num, node_t* args);
node_t* create_if_node(node_t* condition, node_t* then_body, node_t* else_body);
node_t* create_else_node();
node_t* create_while_node(node_t* condition, node_t* body);
node_t* create_var_decl_node(node_t* var, node_t* init);
node_t* create_ret_node(node_t* expr);
node_t* create_break_node();
node_t* create_cond_node();
node_t* destroy_and_null(node_t* node);
void destroy_node(node_t* node);
