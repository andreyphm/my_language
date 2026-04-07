// #include <assert.h>
// #include <stdlib.h>

// #include "parser.h"
// #include "front_end.h"
// #include "font.h"
// #include "dump.h"

// const int STACK_CAPACITY = 1000;
// const int COMMAND_CAPACITY = 100;

// void tree_dump(node_t* const node, const char* const png_file_name, const variable_t* const variables)
// {
//     if (!node)
//     {
//         printf(MAKE_BOLD_RED("Tree is empty, nothing to dump\n"));
//         return;
//     }
//     assert(png_file_name);

//     FILE* txt_file = fopen(TREE_DUMP_TXT, "w");
//     assert(txt_file);

//     fprintf(txt_file, "digraph G {\n");
//     fprintf(txt_file, "    rankdir=TB;\n");
//     fprintf(txt_file, "    node [shape=record, fontname=\"Arial\"];\n");
//     fprintf(txt_file, "    edge [color=\"black\", fontname=\"Arial\", fontsize=10];\n\n");

//     struct stack_element
//     {
//         node_t* node;
//         int index;
//     };

//     struct stack_element* stack = (stack_element*) calloc(STACK_CAPACITY, sizeof(stack_element));
//     assert(stack);

//     int stack_size = 0;
//     int node_index = 0;

//     stack[stack_size].node = node;
//     stack[stack_size].index = node_index++;
//     stack_size++;

//     while (stack_size > 0)
//     {
//         stack_size--;
//         node_t* current = stack[stack_size].node;
//         int current_index = stack[stack_size].index;

//         fprintf(txt_file, "    n%d [label=\"{ <type> type = %s | ", current_index, enum_to_string(current->value->type));
//         switch(current->value->type)
//         {
//             case OP:
//                 fprintf(txt_file, "<val> code = %s | ", operators_array[current->value->data_t.op].name);
//                 fprintf(txt_file, "{ <L> %p | <R> %p } }\", style=filled, fillcolor=\"#b7e5f3ff\"];\n", current->left, current->right);
//                 break;
//             case VAR:
//                 fprintf(txt_file, "<val> num = %d (%s) | ", current->value->data_t.var_number, variables[current->value->data_t.var_number].name);
//                 fprintf(txt_file, "{ <L> %p | <R> %p } }\", style=filled, fillcolor=\"#36ff6fff\"];\n", current->left, current->right);
//                 break;
//             case NUM:
//                 fprintf(txt_file, "<val> val = %lg | ", current->value->data_t.number);
//                 fprintf(txt_file, "{ <L> %p | <R> %p } }\", style=filled, fillcolor=\"#f8c331ff\"];\n", current->left, current->right);
//             case SPEC:
//             default:
//                 break;
//         }

//         if (current->left)
//         {
//             int left_index = node_index++;

//             fprintf(txt_file,"    n%d:L -> n%d [color=\"black\", constraint=true];\n", current_index, left_index);
//             if (stack_size < STACK_CAPACITY)
//             {
//                 stack[stack_size].node = current->left;
//                 stack[stack_size].index = left_index;
//                 stack_size++;
//             }
//         }

//         if (current->right)
//         {
//             int right_index = node_index++;

//             fprintf(txt_file, "    n%d:R -> n%d [color=\"black\", constraint=true];\n", current_index, right_index);
//             if (stack_size < STACK_CAPACITY)
//             {
//                 stack[stack_size].node = current->right;
//                 stack[stack_size].index = right_index;
//                 stack_size++;
//             }
//         }
//     }
//     fprintf(txt_file, "}\n");
//     fclose(txt_file);
//     free(stack);

//     char command[COMMAND_CAPACITY];
//     sprintf(command, "dot -Tpng " TREE_DUMP_TXT " -o %s", png_file_name);
//     system(command);

//     printf(MAKE_BOLD_GREEN("Tree visualization saved to %s\n"), TREE_DUMP_PNG);
//     }

// const char* enum_to_string(type_data type)
// {
//     switch (type)
//     {
//         case OP:
//             return "OP";
//         case VAR:
//             return "VAR";
//         case NUM:
//             return "NUM";
//         case SPEC:
//             return "SPEC";
//         default:
//             return nullptr;
//     }
// }
