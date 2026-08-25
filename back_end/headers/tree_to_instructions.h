#ifndef TREE_TO_INSTRUCTIONS_H
#define TREE_TO_INSTRUCTIONS_H

#include "tree.h"
#include "instructions.h"

void tree_to_instructions(node_t* tree, instruction_list_t* const instruction_list,
                          label_list_t* const label_list, const identifier_t* const identifiers);

#endif // TREE_TO_INSTRUCTIONS_H
