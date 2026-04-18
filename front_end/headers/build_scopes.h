#include "tree.h"

struct var_decl_t
{
    const char* name;
    int decl_id;
    node_t* decl_node;
    var_decl_t* next;
};

struct scope_t
{
    var_decl_t* decl_var;
    scope_t* parent;
};
