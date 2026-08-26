#include "tree.h"
#include "output.h"
#include "font.h"

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
        case PROG_CHILD_COUNT_ERROR:    return "PROG_CHILD_COUNT_ERROR";
        case UNDECLARED_VARIABLE:       return "UNDECLARED_VARIABLE";
        case MULTIPLE_FUNC_DECLARATION: return "MULTIPLE_FUNC_DECLARATION";
        case UNDECLARED_FUNCTION:       return "UNDECLARED_FUNCTION";
        case FUNC_WRONG_NUMBER_OF_ARGS: return "FUNC_WRONG_NUMBER_OF_ARGS";
        case BREAK_OUTSIDE_LOOP:        return "BREAK_OUTSIDE_LOOP";
        case MISSING_ENTRY_POINT:       return "MISSING_ENTRY_POINT";
        case MULTIPLE_ENTRY_POINTS:     return "MULTIPLE_ENTRY_POINTS";
        case OUT_WRONG_NUMBER_OF_ARGS:  return "OUT_WRONG_NUMBER_OF_ARGS";
        case IN_WRONG_NUMBER_OF_ARGS:   return "IN_WRONG_NUMBER_OF_ARGS";
        case SQRT_WRONG_NUMBER_OF_ARGS: return "SQRT_WRONG_NUMBER_OF_ARGS";
        
        default:                        return "NO_ERROR";
    }
}
