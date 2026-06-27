#include <assert.h>

#include "back_end.h"
#include "tree_to_asm.h"
#include "asm_to_binary.h"

void back_end_run(node_t* tree, FILE* const asm_file, FILE* const binary_file, const identifier_t* const identifiers)
{
    assert(tree);
    assert(asm_file);
    assert(identifiers);

    tree_to_asm(tree, asm_file, identifiers);
    fclose(asm_file);

    FILE* asm_read = fopen("output.asm", "r");
    assert(asm_read);
    asm_to_binary(asm_read, binary_file);
    fclose(asm_read);
}
