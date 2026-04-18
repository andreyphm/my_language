#include <stdlib.h>
#include <assert.h>

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

    scope_t* del_scope = *current;
    *current = (*current)->parent;

    free(del_scope);  
}


