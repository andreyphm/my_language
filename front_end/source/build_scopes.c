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

    destroy_scope(current_scope);  
}

void destroy_scope(scope_t* scope)
{
    if (!scope) return;

    var_decl_t* decl_var = scope->decl_var;
    while (decl_var)
    {
        var_decl_t* next = decl_var->next;
        free(decl_var);
        decl_var = next;
    }
    
    func_decl_t* decl_func = scope->decl_func;
    while (decl_func)
    {
        func_decl_t* next = decl_func->next;
        free(decl_func);
        decl_func = next;
    }

    free(scope);
}

error_code declare_var(scope_t* current, const char* var_name, int decl_id, node_t* decl_node)
{
    assert(current);
    assert(decl_node);

    var_decl_t* current_decl = current->decl_var;

    while(current_decl)
    {
        if (decl_id == current_decl->decl_id)
            return MULTIPLE_VAR_DECLARATION;

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

error_code declare_func(scope_t* current, const char* func_name, int decl_id, node_t* decl_node)
{
    assert(current);
    assert(func_name);
    assert(decl_node);

    func_decl_t* current_decl = current->decl_func;

    while(current_decl)
    {
        if (decl_id == current_decl->decl_id)
            return MULTIPLE_FUNC_DECLARATION;

        current_decl = current_decl->next;
    }

    func_decl_t* decl_func = (func_decl_t*) calloc(1, sizeof(func_decl_t));
    assert(decl_func);

    decl_func->name = func_name;
    decl_func->decl_id = decl_id;
    decl_func->decl_node = decl_node;

    decl_func->next = current->decl_func;
    current->decl_func = decl_func;

    return NO_ERROR;
}

func_decl_t* seek_func(const scope_t* const current, const int func_id)
{
    assert(current);

    func_decl_t* current_decl = current->decl_func;

    while(current_decl)
    {
        if (func_id != current_decl->decl_id)
            current_decl = current_decl->next;
        else
            return current_decl;
    }

    if (!current->parent) return nullptr;
    return seek_func(current->parent, func_id); 
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
    assert(identifiers);

    if (!tree) return TREE_NULLPTR;
    if (tree->kind != NODE_PROG) return PROG_NODE_ERROR;

    scope_t* prog_scope = (scope_t*) calloc(1, sizeof(scope_t));
    assert(prog_scope);

    for (size_t i = 0; i < tree->child_count; i++)
    {
        node_t* func_node = tree->children[i];

        error_code error = declare_func(prog_scope,
                                        identifiers[func_node->data_t.id_number].name,
                                        func_node->data_t.id_number,
                                        func_node);
        if (error)
        {
            destroy_scope(prog_scope);
            return error;
        }
    }

    for (size_t i = 0; i < tree->child_count; i++)
    {
       error_code error = analyze_func(tree->children[i], prog_scope, identifiers);
       if (error) 
       {
            destroy_scope(prog_scope);
            return error;
       }
    }

    destroy_scope(prog_scope);
    return NO_ERROR;
}

error_code analyze_func(node_t* func_node, scope_t* parent, const identifier_t* const identifiers)
{
    assert(func_node);
    assert(parent);
    assert(identifiers);

    scope_t* func_scope = (scope_t*) calloc(1, sizeof(scope_t));
    assert(func_scope);

    func_scope->parent = parent;

    for (size_t i = 0; i < func_node->children[0]->child_count; i++)
    {
        error_code error =  declare_var(func_scope, identifiers[func_node->children[0]->children[i]->data_t.id_number].name,
                                        func_node->children[0]->children[i]->data_t.id_number, func_node->children[0]->children[i]);
        if (error) 
        {
            destroy_scope(func_scope);
            return error;
        }
    }

    error_code block_error = analyze_block(func_node->children[1], func_scope, identifiers);
    if (block_error) 
    {
        destroy_scope(func_scope);
        return block_error;
    }

    destroy_scope(func_scope);
    return NO_ERROR;
}

error_code analyze_block(node_t* block_node, scope_t* parent, const identifier_t* const identifiers)
{
    assert(block_node);
    assert(parent);
    assert(identifiers);

    scope_t* block_scope = (scope_t*) calloc(1, sizeof(scope_t));
    assert(block_scope);

    block_scope->parent = parent;

    for (size_t i = 0; i < block_node->child_count; i++)
    {
        error_code error = analyze_op(block_node->children[i], block_scope, identifiers);
        if (error) 
        {
            destroy_scope(block_scope);
            return error;
        }
    }

    destroy_scope(block_scope);
    return NO_ERROR;
}

error_code analyze_op(node_t* op_node, scope_t* parent, const identifier_t* const identifiers)
{
    assert(op_node);
    assert(parent);
    assert(identifiers);

    switch(op_node->kind)
    {
        case NODE_IF:
            return analyze_if(op_node, parent, identifiers);

        case NODE_WHILE:
            return analyze_while(op_node, parent, identifiers);

        case NODE_VAR_DECL:
            return analyze_var_decl(op_node, parent, identifiers);

        case NODE_RET:
            if (op_node->child_count > 0)
                return analyze_expr(op_node->children[0], parent, identifiers);
            return NO_ERROR;

        case NODE_NUM:
        case NODE_VAR:
        case NODE_OP:
        case NODE_CALL:
            return analyze_expr(op_node, parent, identifiers);

        case NODE_BREAK:
        default:
            return NO_ERROR;
    }
}

error_code analyze_if(node_t* if_node, scope_t* parent, const identifier_t* const identifiers)
{
    assert(if_node);
    assert(parent);
    assert(identifiers);

    scope_t* if_scope = (scope_t*) calloc(1, sizeof(scope_t));
    assert(if_scope);
    if_scope->parent = parent;

    error_code cond_error = analyze_expr(if_node->children[0], if_scope, identifiers);
    if (cond_error) 
    {
        destroy_scope(if_scope);
        return cond_error;
    }

    error_code body_error = analyze_block(if_node->children[1], if_scope, identifiers);
    if (body_error)
    {
        destroy_scope(if_scope);
        return body_error;
    }

    if (if_node->child_count > 2)
    {
        error_code else_error = analyze_block(if_node->children[2], if_scope, identifiers);
        if (else_error) 
        {
            destroy_scope(if_scope);
            return else_error;
        }
    }

    destroy_scope(if_scope);
    return NO_ERROR;
}

error_code analyze_while(node_t* while_node, scope_t* parent, const identifier_t* const identifiers)
{
    assert(while_node);
    assert(parent);
    assert(identifiers);

    scope_t* while_scope = (scope_t*) calloc(1, sizeof(scope_t));
    assert(while_scope);
    while_scope->parent = parent;

    error_code cond_error = analyze_expr(while_node->children[0], while_scope, identifiers);
    if (cond_error) 
    {
        destroy_scope(while_scope);
        return cond_error;
    }

    error_code body_error = analyze_block(while_node->children[1], while_scope, identifiers);
    if (body_error)
    {
        destroy_scope(while_scope);
        return body_error;
    }

    destroy_scope(while_scope);
    return NO_ERROR;
}

error_code analyze_var_decl(node_t* var_decl_node, scope_t* current, const identifier_t* const identifiers)
{
    assert(var_decl_node);
    assert(current);
    assert(identifiers);

    node_t* var_node = var_decl_node->children[0];
    int var_id = var_node->data_t.id_number;

    error_code var_decl_error = declare_var(current, identifiers[var_id].name, var_id, var_decl_node);
    if (var_decl_error) return var_decl_error;

    if (var_decl_node->child_count > 1)
    {
        error_code expr_error = analyze_expr(var_decl_node->children[1], current, identifiers);
        if (expr_error) return expr_error;
    }
    
    return NO_ERROR;
}

error_code analyze_expr(node_t* expr_node, scope_t* current, const identifier_t* const identifiers)
{
    assert(expr_node);
    assert(current);
    assert(identifiers);

    switch(expr_node->kind)
    {
        case NODE_NUM: 
            return NO_ERROR;

        case NODE_VAR: 
            if (!seek_var(current, expr_node->data_t.id_number))
                return UNDECLARED_VARIABLE;
            return NO_ERROR;

        case NODE_OP:
            for (size_t i = 0; i < expr_node->child_count; i++)
            {
                error_code error = analyze_expr(expr_node->children[i], current, identifiers);
                if (error) return error;
            }
            return NO_ERROR;

        case NODE_CALL:
            if (!seek_func(current, expr_node->data_t.id_number))
                    return UNDECLARED_FUNCTION;
            for (size_t i = 0; i < expr_node->children[0]->child_count; i++)
            {
                error_code error = analyze_expr(expr_node->children[0]->children[i], current, identifiers);
                if (error) return error;
            }
            return NO_ERROR;

        default: 
            return NO_ERROR;
    }
}

