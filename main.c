#include "front_end.h"
#include "input.h"
#include "tokenization.h"
#include "parser.h"
#include "output.h"
#include "build_scopes.h"
#include "middle_end.h"
#include "font.h"
#include "back_end.h"

void program_complete(identifier_t** identifiers_ptr, node_t** node_ptr, FILE* input_file);

int main(int argc, const char* argv[])
{
    FILE* input_file = nullptr;
    FILE* output_file = nullptr;
    check_files(&input_file, &output_file, argc, argv);

    identifier_t* identifiers = nullptr;
    node_t* tree = front_end_run(input_file, &identifiers); 
    if (!tree)
    {
        fclose(output_file);
        return 1;
    }

    middle_end_run(tree);
    
    tree_dump(tree, TREE_DUMP_PNG, identifiers);

    back_end_run(tree, output_file, identifiers);

    program_complete(&identifiers, &tree, input_file);
}

void program_complete(identifier_t** identifiers_ptr, node_t** node_ptr, FILE* input_file)
{
    if (*identifiers_ptr) identifiers_destroy(identifiers_ptr);
    destroy_node(*node_ptr);
    fclose(input_file);
    printf(MAKE_BOLD("COMMIT GITHUB\n"));
}
