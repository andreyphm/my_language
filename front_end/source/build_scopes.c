#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "front_end.h"
#include "build_scopes.h"

void enter_scope(scope_t** current)
{
    assert(current);
    assert(*current);

    scope_t* new_scope = (scope_t*) calloc(1, sizeof(scope_t));
    assert(new_scope);

    new_scope->parent = *current;

    *current = new_scope;
}

void exit_scope(scope_t** current)
{
    assert(current);
    assert(*current);

    scope_t* current_scope = *current;
    *current = (*current)->parent;

    free(current_scope);  
}

error_code declare_var(scope_t* current, const char* var_name, int decl_id, node_t* decl_node)
{
    assert(current);

    var_decl_t* current_decl = current->decl_var;

    while(current_decl)
    {
        if (decl_id == current_decl->decl_id)
            return RE_DECLARING;

        current_decl = current_decl->next;
    }

    var_decl_t* decl_var = (var_decl_t*) calloc(1, sizeof(var_decl_t));
    assert(decl_var);

    decl_var->name = var_name;
    decl_var->decl_id = decl_id;
    decl_var->decl_node = decl_node;

    decl_var->next = current->decl_var;
    current->decl_var = decl_var;

    return NO_ERROR;
}

var_decl_t* seek_var(scope_t* current, int var_id)
{
    assert(current);

    var_decl_t* current_decl = current->decl_var;

    while(current_decl)
    {
        if (var_id != current_decl->decl_id)
            current_decl = current_decl->next;
        else
            return current_decl;
    }

    if (!current->parent) return nullptr;
    return seek_var(current->parent, var_id);
}

void build_scopes(node_t* tree, const identifier_t* const identifiers)
{
    scope_t* first_scope = (scope_t*) calloc(1, sizeof(scope_t));
    assert(first_scope);

    for (size_t i = 0; i < tree->child_count; i++)
       analyze_func(tree->children[i], first_scope, identifiers); 
}

void analyze_func(node_t* func_node, scope_t* parent, const identifier_t* const identifiers)
{
    assert(func_node);
    assert(parent);

    scope_t* func_scope = (scope_t*) calloc(1, sizeof(scope_t));
    assert(func_scope);

    func_scope->parent = parent;

    for (size_t i = 0; i < func_node->children[0]->child_count; i++)
    {
        declare_var(func_scope, 
                    identifiers[func_node->children[0]->children[i]->data_t.id_number].name,
                    func_node->children[0]->children[i]->data_t.id_number, 
                    func_node->children[0]->children[i]);
    }

    analyze_block(func_node->children[1], func_scope, identifiers);
}

void analyze_block(node_t* block_node, scope_t* parent, const identifier_t* const identifiers)
{
    assert(block_node);
    assert(parent);

    scope_t* block_scope = (scope_t*) calloc(1, sizeof(scope_t));
    assert(block_scope);

    block_scope->parent = parent;

    for (size_t i = 0; i < block_node->child_count; i++)
        analyze_op(block_node->children[i], block_scope, identifiers);
}

void analyze_op(node_t* op_node, scope_t* parent, const identifier_t* const identifiers)
{
    assert(op_node);
    assert(parent);


}
