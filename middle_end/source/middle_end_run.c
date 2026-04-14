#include "middle_end.h"

void middle_end_run(node_t* tree)
{
    for (bool simplifications = true; simplifications;)
    {
        simplifications = false;
        tree = simplify_node(tree, &simplifications);
    }
}
