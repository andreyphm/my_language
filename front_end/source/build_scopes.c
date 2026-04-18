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

error_code declare_var(scope_t* current, int decl_id, const char* var_name, node_t* decl_node)
{
    assert(current);

    var_decl_t* current_decl = current->decl_var;

    while(current_decl)
    {
        if (!strcmp(var_name, current_decl->name))
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
