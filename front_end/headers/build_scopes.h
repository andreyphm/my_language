#include "tree.h"

struct var_decl_t
{
    const char* name;
    int decl_id;
    node_t* decl_node;
};

struct scope_t
{
    var_decl_t* decl_variables;
    int decl_variables_number;
    scope_t* parent;
};
