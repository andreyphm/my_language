#include "front_end.h"
#include "input.h"
#include "tokenization.h"
#include "parser.h"
#include "output.h"
#include "dump.h"

int main(int argc, const char* argv[])
{
    FILE* input_file = nullptr;
    FILE* output_file = nullptr;
    check_files(&input_file, &output_file, argc, argv);

    error_code error = NO_ERROR;
    node_t* node = nullptr;
    identifier_t* identifiers = nullptr;
    list_t list = {nullptr, nullptr, nullptr};

    error = file_to_tokens(&identifiers, input_file, &list);
    if (error) 
    {
        error_message(error);
        program_complete(&identifiers, &node, input_file);
        return 0;
    }
    list_dump(&list, LIST_DUMP_TXT, LIST_DUMP_PNG, identifiers);

    error = tokens_to_tree(&list, &node);
    if (error) 
    {
        error_message(error);
        program_complete(&identifiers, &node, input_file);
        return 0;
    }
    tree_dump(node, TREE_DUMP_PNG, identifiers);

    program_complete(&identifiers, &node, input_file);
}
