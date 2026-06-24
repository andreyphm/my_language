#ifndef BACK_END_H
#define BACK_END_H

#include "tree.h"

void back_end_run(node_t* tree, FILE* const output_file, const identifier_t* const identifiers);

#endif // BACK_END_H
