#include <assert.h>

#include "back_end.h"
#include "tree_to_asm.h"

void back_end_run(node_t* tree, FILE* const output_file, const identifier_t* const identifiers)
{
    assert(tree);
    assert(output_file);
    assert(identifiers);

    tree_to_asm(tree, output_file, identifiers);

    // asm_to_binary(output_file);
}
