#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "output.h"
#include "macros.h"
#include "font.h"
#include "parser.h"

void destroy_tree_and_id_array(identifier_t** identifiers_ptr, node_t** node_ptr)
{
    if (*identifiers_ptr) identifiers_destroy(identifiers_ptr);
    destroy_node(*node_ptr);
}

void error_message(error_code error)
{
    printf(MAKE_BOLD_RED("Program exit with fail: %s\n"), error_to_string(error));
}

const char* error_to_string(error_code error)
{
    switch(error)
    {
        case SYNTAX_ERROR:              return "SYNTAX_ERROR";
        case TREE_NULLPTR:              return "TREE_NULLPTR";
        case MULTIPLE_VAR_DECLARATION:  return "MULTIPLE_VAR_DECLARATION";
        case PROG_NODE_ERROR:           return "PROG_NODE_ERROR";
        case UNDECLARED_VARIABLE:       return "UNDECLARED_VARIABLE";
        case MULTIPLE_FUNC_DECLARATION: return "MULTIPLE_FUNC_DECLARATION";
        case UNDECLARED_FUNCTION:       return "UNDECLARED_FUNCTION";
        
        default:                        return "NO_ERROR";
    }
}
