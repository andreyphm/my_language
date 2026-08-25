#include <assert.h>
#include <string.h>

#include "back_end.h"
#include "tree_to_instructions.h"
#include "instructions_to_binary.h"
#include "instructions_to_nasm.h"
#include "instructions_dump.h"

static bool tree_uses_stdlib(const node_t* tree, const identifier_t* identifiers);

void back_end_run(node_t* tree, FILE* output_file,
                  const identifier_t* const identifiers, back_end_mode mode)
{
    assert(tree);
    assert(output_file);
    assert(identifiers);

    instruction_list_t instructions = {};
    label_list_t labels = {};
    tree_to_instructions(tree, &instructions, &labels, identifiers);

    instructions_dump(&instructions, &labels,
                      INSTRUCTIONS_DUMP_TXT, INSTRUCTIONS_DUMP_SVG);

    bool use_stdlib = tree_uses_stdlib(tree, identifiers);
    if (mode == BACK_END_NASM)
        instructions_to_nasm(&instructions, &labels, output_file,
                             use_stdlib ? STDLIB_SOURCE_FILE : nullptr);
    else
        instructions_to_binary(&instructions, &labels, output_file,
                               use_stdlib ? STDLIB_BINARY_FILE : nullptr);

    instruction_list_destroy(&instructions);
    label_list_destroy(&labels);
}

static bool tree_uses_stdlib(const node_t* tree, const identifier_t* identifiers)
{
    assert(tree);
    assert(identifiers);
    assert(tree->child_count >= 1);

    const node_t* includes = tree->children[0];
    for (size_t i = 0; i < includes->child_count; i++)
    {
        int identifier_id = includes->children[i]->data_t.include.id_number;
        if (!strcmp(identifiers[identifier_id].name, "my_stdlib"))
            return true;
    }

    return false;
}
