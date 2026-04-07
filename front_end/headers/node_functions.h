#include "front_end.h"

node_t* create_node(const type_data type, data_union data, node_t* left, node_t* right);
void destroy_node(node_t* node);

node_t* copy_node(node_t* node);
// node_t* simplify_node(node_t* node, bool* simplifications_ptr);
