#include "back_end.h"
#include "font.h"

void back_end_run(node_t* tree, FILE* const output_file, const identifier_t* const identifiers)
{
    gen_prog(tree, output_file, identifiers);

    printf(MAKE_BOLD_GREEN("Tree to NASM successful\n"));
}

void gen_prog(node_t* prog_node, FILE* const output_file, const identifier_t* const identifiers)
{
    for (size_t i = 0; i < prog_node->child_count; i++)
    {
        gen_func(prog_node->children[i], output_file, identifiers);
    }
}

void gen_func(node_t* func_node, FILE* const output_file, const identifier_t* const identifiers)
{
    size_t body_local_vars = count_local_vars(func_node->children[1]);

    fprintf(output_file, ";==========FUNCTION %s==========\n"
                         "push rbp\n"
                         "mov rbp, rsp\n"
                         "sub rsp, %zu\n",
                         identifiers[func_node->data_t.id_number].name,
                         (func_node->children[0]->child_count + body_local_vars + 1) * sizeof(double));

    // gen_block(func_node->children[0]);
    // gen_block(func_node->children[1]);
}

// void gen_block(node_t* block_node)
// {
//     for (size_t i = 0; i < block_node->child_count; i++)
//         gen_op(block_node->children[i]);
// }

// void gen_op(node_t* op_node)
// {
//     switch (op_node->kind)
//     {
//         case NODE_RET:

//     }
// }

size_t count_local_vars(node_t* current)
{
    size_t local_vars = 0;
    if (current->child_count <= 0) return 0;
    
    for (size_t i = 0; i < current->child_count; i++)
        local_vars += count_local_vars(current->children[i]);

    if (current->kind == NODE_VAR_DECL) return 1;

    return local_vars;
}
