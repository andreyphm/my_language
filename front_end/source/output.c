#include <stdlib.h>
#include <string.h>

#include "node_functions.h"
#include "output.h"
#include "macros.h"
#include "font.h"
#include "dump.h"
#include "parser.h"

void program_complete(variable_t** variables_ptr, node_t** node_ptr, FILE* input_file)
{
    if (*variables_ptr) variables_destroy(variables_ptr);
    destroy_node(*node_ptr);
    fclose(input_file);
    printf(MAKE_BOLD("Program completed. COMMIT GITHUB\n"));
}

void error_message(error_code error)
{
    printf(MAKE_BOLD_RED("Program exit with fail: %s"), error_to_string(error));
}

const char* error_to_string(error_code error)
{
    switch(error)
    {
        case SYNTAX_ERROR:
            return "SYNTAX ERROR";
        case TREE_NULLPTR:
            return "TREE NULLPTR";
        case NO_ERROR:
        default:
            return nullptr;
    }
}
