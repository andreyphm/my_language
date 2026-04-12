#include <stdlib.h>
#include <string.h>

#include "node_functions.h"
#include "output.h"
#include "macros.h"
#include "font.h"
#include "parser.h"

void program_complete(identifier_t** identifiers_ptr, node_t** node_ptr, FILE* input_file)
{
    if (*identifiers_ptr) identifiers_destroy(identifiers_ptr);
    destroy_node(*node_ptr);
    fclose(input_file);
    printf(MAKE_BOLD("COMMIT GITHUB\n"));
}

void error_message(error_code error)
{
    printf(MAKE_BOLD_RED("Program exit with fail: %s\n"), error_to_string(error));
}

const char* error_to_string(error_code error)
{
    switch(error)
    {
        case SYNTAX_ERROR:
            return "SYNTAX_ERROR";
        case TREE_NULLPTR:
            return "TREE_NULLPTR";
        case NO_ERROR:
        default:
            return "NO_ERROR";
    }
}
