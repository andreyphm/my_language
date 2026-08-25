#ifndef INSTRUCTIONS_TO_NASM_H
#define INSTRUCTIONS_TO_NASM_H

#include <stdio.h>

#include "instructions.h"

void instructions_to_nasm(const instruction_list_t* instructions, const label_list_t* labels,
                          FILE* output_file, const char* stdlib_source_path);

#endif // INSTRUCTIONS_TO_NASM_H
