#include "front_end.h"
#include "input.h"
#include "tokenization.h"
#include "parser.h"
#include "output.h"
#include "tree.h"
#include "list_dump.h"
#include "middle_end.h"

node_t* front_end_run(FILE* input_file, identifier_t** identifiers)
{
    error_code error = NO_ERROR;
    node_t* tree = nullptr;
    list_t list = {nullptr, nullptr, nullptr};

    error = file_to_tokens(identifiers, input_file, &list);
    if (error) 
    {
        error_message(error);
        destroy_tree_and_id_array(identifiers, &tree);
        fclose(input_file);
        return 0;
    }
    list_dump(&list, LIST_DUMP_TXT, LIST_DUMP_PNG, *identifiers);

    error = tokens_to_tree(&list, &tree);
    if (error) 
    {
        error_message(error);
        destroy_tree_and_id_array(identifiers, &tree);
        fclose(input_file);
        return nullptr;
    }

    build_scopes(&tree);

    return tree;
}
