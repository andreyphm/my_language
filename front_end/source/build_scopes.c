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

var_decl_t* seek_var(const scope_t* const current, const int var_id)
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

error_code build_scopes(node_t* tree, const identifier_t* const identifiers)
{
    scope_t* first_scope = (scope_t*) calloc(1, sizeof(scope_t));
    assert(first_scope);

    for (size_t i = 0; i < tree->child_count; i++)
       analyze_func(tree->children[i], first_scope, identifiers); 
}

error_code analyze_func(node_t* func_node, scope_t* parent, const identifier_t* const identifiers)
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

error_code analyze_block(node_t* block_node, scope_t* parent, const identifier_t* const identifiers)
{
    assert(block_node);
    assert(parent);

    scope_t* block_scope = (scope_t*) calloc(1, sizeof(scope_t));
    assert(block_scope);

    block_scope->parent = parent;

    for (size_t i = 0; i < block_node->child_count; i++)
        analyze_op(block_node->children[i], block_scope, identifiers);
}

error_code analyze_op(node_t* op_node, scope_t* parent, const identifier_t* const identifiers)
{
    assert(op_node);
    assert(parent);

    scope_t* op_scope = parent;
    switch(op_node->kind)
    {
        case NODE_IF:       return analyze_if(op_node, parent, identifiers);
        case NODE_WHILE:    return analyze_while(op_node, parent, identifiers);
        case NODE_VAR_DECL:
            declare_var(op_scope, identifiers[op_node->data_t.id_number].name,op_node->data_t.id_number, op_node);
            break;

        default: 
    }
}

error_code analyze_if(node_t* if_node, scope_t* parent, const identifier_t* const identifiers)
{
    assert(if_node);
    assert(parent);

    scope_t* if_scope = (scope_t*) calloc(1, sizeof(scope_t));
    assert(if_scope);
    if_scope->parent = parent;

    error_code cond_error = analyze_op(if_node->children[0], if_scope, identifiers);
    if (cond_error) return cond_error;

    error_code body_error = analyze_block(if_node->children[1], if_scope, identifiers);
    if (body_error) return body_error;

    if (if_node->children[2])
    {
        error_code else_error = analyze_block(if_node->children[2], if_scope, identifiers);
        if (else_error) return else_error;
    }

    return NO_ERROR;
}

error_code analyze_while(node_t* while_node, scope_t* parent, const identifier_t* const identifiers)
{
    assert(while_node);
    assert(parent);

    scope_t* while_scope = (scope_t*) calloc(1, sizeof(scope_t));
    assert(while_scope);
    while_scope->parent = parent;

    error_code cond_error = analyze_op(while_node->children[0], parent, identifiers);
    if (cond_error) return cond_error;

    error_code body_error = analyze_block(while_node->children[1], while_scope, identifiers);
    if (body_error) return body_error;

    return NO_ERROR;
}

error_code analyze_var_decl(node_t* var_decl_node, scope_t* parent, const identifier_t* const identifiers)
{
    
}
