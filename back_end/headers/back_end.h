#ifndef BACK_END_H
#define BACK_END_H

#include "tree.h"

struct func_stack_frame
{
    size_t frame_size;
    
};

void back_end_run(node_t* tree, FILE* const output_file, const identifier_t* const identifiers);

void gen_prog(node_t* prog_node, FILE* const output_file, const identifier_t* const identifiers);
void gen_func(node_t* func_node, FILE* const output_file, const identifier_t* const identifiers);

size_t count_local_vars(node_t* current);

#endif // BACK_END_H
