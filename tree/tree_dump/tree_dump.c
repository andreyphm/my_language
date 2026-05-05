#include <assert.h>
#include <stdlib.h>

#include "parser.h"
#include "front_end.h"
#include "font.h"

const int STACK_CAPACITY = 1000;
const int COMMAND_CAPACITY = 100;

const char* node_kind_to_str(node_kind kind);

void tree_dump(node_t* const node, const char* const png_file_name, const identifier_t* const identifiers)
{
    if (!node)
    {
        printf(MAKE_BOLD_RED("Tree is empty, nothing to dump\n"));
        return;
    }
    assert(png_file_name);

    FILE* txt_file = fopen(TREE_DUMP_TXT, "w");
    assert(txt_file);

    fprintf(txt_file, "digraph G {\n");
    fprintf(txt_file, "    rankdir=TB;\n");
    fprintf(txt_file, "    node [shape=record, fontname=\"Arial\"];\n");
    fprintf(txt_file, "    edge [color=\"black\", fontname=\"Arial\", fontsize=10];\n\n");

    struct stack_element
    {
        node_t* node;
        int index;
    };

    struct stack_element* stack = (stack_element*)calloc(STACK_CAPACITY, sizeof(struct stack_element));
    assert(stack);

    int stack_size = 0;
    int node_index = 0;

    stack[stack_size].node = node;
    stack[stack_size].index = node_index++;
    stack_size++;

    while (stack_size > 0)
    {
        stack_size--;
        node_t* current = stack[stack_size].node;
        int current_index = stack[stack_size].index;

        fprintf(txt_file, "    n%d [label=\"{ <type> %s", current_index, node_kind_to_str(current->kind));

        switch (current->kind)
        {
            case NODE_PROG:
                fprintf(txt_file, "}\", style=filled, fillcolor=\"#8482fbff\"];\n");
                break;

            case NODE_OP:
                fprintf(txt_file, " | <val> %s}\", style=filled, fillcolor=\"#b7e5f3ff\"];\n",
                        operators_array[current->data_t.op].name);
                break;

            case NODE_FUNC:
            case NODE_CALL:
                fprintf(txt_file, " | <val> id = %d (%s)}\", style=filled, fillcolor=\"#f577e8ff\"];\n",
                        current->data_t.function.id_number,
                        identifiers ? identifiers[current->data_t.function.id_number].name : "UNKNOWN");
                break;

            case NODE_VAR:
                fprintf(txt_file, " | <val> uid = %d (%s)}\", style=filled, fillcolor=\"#36ff6fff\"];\n",
                        current->data_t.variable.unique_id,
                        identifiers ? identifiers[current->data_t.variable.id_number].name : "UNKNOWN");
                break;

            case NODE_VAR_DECL:
                fprintf(txt_file, " | <uid> uid = %d}\", style=filled, fillcolor = \"#f1e724ff\"];\n",
                        current->data_t.variable.unique_id);
                break;

            case NODE_NUM:
                fprintf(txt_file, " | <val> value = %lg}\", style=filled, fillcolor=\"#f8c331ff\"];\n", current->data_t.number);
                break;

            case NODE_RET:
            case NODE_BREAK:
                fprintf(txt_file, "}\", style=filled, fillcolor=\"#f07f7fff\"];\n");
                break;

            case NODE_BODY:
            case NODE_ARGS:
            case NODE_COND:
                fprintf(txt_file, "}\", style=filled, fillcolor=\"#09be76ff\"];\n");
                break;

            default:
                fprintf(txt_file, "}\", style=filled, fillcolor=\"#f1e724ff\"];\n");
                break;
        }

        for (size_t i = 0; i < current->child_count; i++)
        {
            node_t* child = current->children[i];
            if (!child) continue;

            int child_index = node_index++;

            fprintf(txt_file, "    n%d -> n%d [color=\"black\", constraint=true];\n",
                    current_index, child_index);

            if (stack_size < STACK_CAPACITY)
            {
                stack[stack_size].node = child;
                stack[stack_size].index = child_index;
                stack_size++;
            }
        }
    }

    fprintf(txt_file, "}\n");
    fclose(txt_file);
    free(stack);

    char command[COMMAND_CAPACITY] = {};
    sprintf(command, "dot -Tpng " TREE_DUMP_TXT " -o %s", png_file_name);
    system(command);

    printf(MAKE_BOLD_GREEN("Tree visualization saved to %s\n"), png_file_name);
}

const char* node_kind_to_str(node_kind kind)
{
    switch (kind)
    {
        case NODE_OP:        return "OP";
        case NODE_NUM:       return "NUM";
        case NODE_VAR:       return "VAR";
        case NODE_FUNC:      return "FUNC";
        case NODE_CALL:      return "CALL";
        case NODE_BODY:      return "BODY";
        case NODE_ARGS:      return "ARGS";
        case NODE_IF:        return "IF";
        case NODE_ELSE:      return "ELSE";
        case NODE_WHILE:     return "WHILE";
        case NODE_VAR_DECL:  return "VAR_DECL";
        case NODE_RET:       return "RET";
        case NODE_BREAK:     return "BREAK";
        case NODE_PROG:      return "PROGRAM";
        case NODE_COND:      return "COND";
        default:             return "UNKNOWN";
    }
}
