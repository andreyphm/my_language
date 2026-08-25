#include "front_end.h"
#include "input.h"
#include "middle_end.h"
#include "font.h"
#include "back_end.h"

#include <string.h>

static void program_complete(identifier_t** identifiers_ptr, node_t** node_ptr, FILE* input_file);
static back_end_mode get_back_end_mode(int argc, const char* argv[]);

int main(int argc, const char* argv[])
{
    FILE* input_file = nullptr;
    FILE* output_file = nullptr;
    back_end_mode mode = get_back_end_mode(argc, argv);
    int files_argc = argc == 4 ? 3 : argc;
    check_files(&input_file, &output_file, files_argc, argv);

    identifier_t* identifiers = nullptr;
    node_t* tree = front_end_run(input_file, &identifiers); 
    if (!tree)
    {
        fclose(output_file);
        return 1;
    }

    middle_end_run(tree);
    
    tree_dump(tree, TREE_DUMP_SVG, identifiers);

    back_end_run(tree, output_file, identifiers, mode);
    fclose(output_file);

    program_complete(&identifiers, &tree, input_file);
}

static back_end_mode get_back_end_mode(int argc, const char* argv[])
{
    if (argc != 4)
    {
        printf(MAKE_BOLD_GREEN("NASM mode has been selected.\n"));
        return BACK_END_NASM;
    }

    if (!strcmp(argv[3], "bin") || !strcmp(argv[3], "binary"))
    {
        printf(MAKE_BOLD_GREEN("Binary mode has been selected.\n"));
        return BACK_END_BINARY;
    }

    if (!strcmp(argv[3], "asm") || !strcmp(argv[3], "nasm"))
    {
        printf(MAKE_BOLD_GREEN("NASM mode has been selected.\n"));
        return BACK_END_NASM;
    }

    fprintf(stderr, "Unknown backend mode '%s'. Use 'nasm' or 'bin'.\n", argv[3]);
    return BACK_END_NASM;
}

static void program_complete(identifier_t** identifiers_ptr, node_t** node_ptr, FILE* input_file)
{
    if (*identifiers_ptr) identifiers_destroy(identifiers_ptr);
    destroy_node(*node_ptr);
    fclose(input_file);
    printf(MAKE_BOLD("COMMIT GITHUB\n"));
}
