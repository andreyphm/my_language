#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "tokenization.h"
#include "list_dump.h"
#include "font.h"

void list_dump(list_t* const list, const char* const txt_file_name, const char* const png_file_name, 
                                                                const identifier_t* const identifiers)
{
    assert(list);
    assert(txt_file_name);
    assert(png_file_name);

    FILE* txt_file = fopen(txt_file_name, "w");
    fprintf(txt_file, "digraph structs\n{\nrankdir = LR;\ngraph[bgcolor=\"#e0e0e9ff\"];\n");

    int node_number = 1;
    for (list->current = list->head; ; node_number++)
    {
        switch(list->current->type)
        {
            case OP:
                fprintf(txt_file, "node_%d [style=filled, penwidth = 3, fillcolor=\"#b7e5f3ff\","
                   "color = \"#3f6969ff\", shape=record, label= \" ", node_number);
                fprintf(txt_file, "TYPE = OP | OP_CODE = %s | ", operators_array[list->current->data_t.op].name);
                break;
            case ID:
                fprintf(txt_file, "node_%d [style=filled, penwidth = 3, fillcolor=\"#36ff6fff\","
                   "color = \"#3f6969ff\", shape=record, label= \" ", node_number);
                fprintf(txt_file, "TYPE = ID | ID_NUM = %d (%s) | ", list->current->data_t.id_number, 
                                                                        identifiers[list->current->data_t.id_number].name);
                break;
            case KEYWORD:
                fprintf(txt_file, "node_%d [style=filled, penwidth = 3, fillcolor=\"#f1e724ff\","
                   "color = \"#3f6969ff\", shape=record, label= \" ", node_number);
                fprintf(txt_file, "TYPE = KEYWORD | KEYWORD_CODE = %s | ", keywords_array[list->current->data_t.keyword].name);
                break;
            case NUM:
                fprintf(txt_file, "node_%d [style=filled, penwidth = 3, fillcolor=\"#f8c331ff\","
                   "color = \"#3f6969ff\", shape=record, label= \" ", node_number);
                fprintf(txt_file, "TYPE = NUM | VALUE = %lg | ", list->current->data_t.number);
                break;  
            case SPEC:
                fprintf(txt_file, "node_%d [style=filled, penwidth = 3, fillcolor=\"#f673e9ff\","
                   "color = \"#3f6969ff\", shape=record, label= \" ", node_number);
                fprintf(txt_file, "TYPE = SPEC | VALUE = %s | ", spec_to_str(list->current->data_t.spec));
            default:
                break;
        }

        fprintf(txt_file, "{line = %zu | column = %zu | length = %zu} |\n",
                          list->current->position.line_number,
                          list->current->position.column_number,
                          list->current->position.length);
        fprintf(txt_file, "ADDRESS = %p |\n", list->current);
        fprintf(txt_file, "{next = %p}\" ];\n", list->current->next);

        if (list->current == list->tail)
            break;
        list->current = list->current->next;
    }

    int list_capacity = node_number;
    fprintf(txt_file, "{\nedge[color = \"#149b5aff\", weight = 1000];\n");

    for (node_number = 1; node_number < list_capacity; node_number++)
        fprintf(txt_file, "node_%d -> node_%d\n", node_number, node_number + 1);

    fprintf(txt_file, "}\n");
    fprintf(txt_file, "}");
    fclose(txt_file);

    char command[1000];
    sprintf(command, "dot %s -T png -o %s", txt_file_name, png_file_name);
    system(command);

    printf(MAKE_BOLD_GREEN("List visualization saved to %s\n"), LIST_DUMP_PNG);
}

const char* spec_to_str(spec_code spec)
{
    switch(spec)
    {
        case LEFT_BRACE:    return "LEFT_BRACE";
        case RIGHT_BRACE:   return "RIGHT_BRACE";
        case LEFT_PAREN:    return "LEFT_PAREN";
        case RIGHT_PAREN:   return "RIGHT_PAREN";
        case SEMMICOLON:    return "SEMMICOLON";
        case COMMA:         return "COMMA";
        case PROGRAM_END:   return "PROGRAM_END";

        default:            return "UNKNOWN";
    }
}
