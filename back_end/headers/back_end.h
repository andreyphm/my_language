#ifndef BACK_END_H
#define BACK_END_H

#include "tree.h"

#define BINARY_FILE "output.bin"

void back_end_run(node_t* tree, FILE* const asm_file, FILE* const binary_file, const identifier_t* const identifiers);

#endif // BACK_END_H
