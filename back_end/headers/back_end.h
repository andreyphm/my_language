#ifndef BACK_END_H
#define BACK_END_H

#include "tree.h"

struct func_stack_frame
{
    size_t frame_size;
    
};

void back_end_run(node_t* tree);

#endif // BACK_END_H
