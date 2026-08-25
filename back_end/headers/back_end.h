#ifndef BACK_END_H
#define BACK_END_H

#include "tree.h"

#define BINARY_FILE "output.bin"
#define STDLIB_SOURCE_FILE "my_stdlib.asm"
#define STDLIB_BINARY_FILE "build/my_stdlib.bin"

enum back_end_mode
{
    BACK_END_NASM,
    BACK_END_BINARY
};

void back_end_run(node_t* tree, FILE* output_file,
                  const identifier_t* identifiers, back_end_mode mode);

#endif // BACK_END_H
