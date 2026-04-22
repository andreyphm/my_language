#include "tree.h"

struct var_decl_t
{
    const char* name;
    int decl_id;
    node_t* decl_node;
    var_decl_t* next;
};

struct func_decl_t
{
    const char* name;
    int decl_id;
    node_t* decl_node;
    func_decl_t* next;
};

struct scope_t
{
    var_decl_t* decl_var;
    func_decl_t* decl_func;
    scope_t* parent;
};

void enter_scope(scope_t** current);
void exit_scope(scope_t** current);
void destroy_scope(scope_t* scope);
error_code declare_var(scope_t* current, const char* var_name, int decl_id, node_t* decl_node);
error_code declare_func(scope_t* current, const char* func_name, int decl_id, node_t* decl_node);
var_decl_t* seek_var(const scope_t* const current, const int var_id);
func_decl_t* seek_func(const scope_t* const current, const int func_id);

error_code build_scopes(node_t* tree, const identifier_t* const identifiers);
error_code analyze_func(node_t* func_node, scope_t* parent, const identifier_t* const identifiers);
error_code analyze_block(node_t* block_node, scope_t* parent, const identifier_t* const identifiers);
error_code analyze_op(node_t* op_node, scope_t* parent, const identifier_t* const identifiers);
error_code analyze_if(node_t* if_node, scope_t* parent, const identifier_t* const identifiers);
error_code analyze_while(node_t* while_node, scope_t* parent, const identifier_t* const identifiers);
error_code analyze_var_decl(node_t* var_decl_node, scope_t* current, const identifier_t* const identifiers);
error_code analyze_expr(node_t* expr_node, scope_t* current, const identifier_t* const identifiers);
