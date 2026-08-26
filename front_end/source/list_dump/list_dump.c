#include <stdlib.h>
#include <assert.h>

#include "list_dump.h"
#include "font.h"

static const size_t TOKENS_PER_ROW   = 20;
static const size_t TOKEN_NODE_WIDTH = 260;
static const size_t FIELD_NAME_WIDTH = 80;
static const size_t FIELD_VALUE_WIDTH = TOKEN_NODE_WIDTH - FIELD_NAME_WIDTH;

static const char* token_type_to_str(type_data type);
static const char* token_color(type_data type);
static void print_html_escaped(FILE* file, const char* text);

void list_dump(list_t* const list, const char* const txt_file_name, const char* const svg_file_name,
                                                                const identifier_t* const identifiers)
{
    assert(list);
    assert(txt_file_name);
    assert(svg_file_name);

    FILE* txt_file = fopen(txt_file_name, "w");
    assert(txt_file);
    fprintf(txt_file, "digraph structs\n{\nrankdir = TB;\n"
                      "graph[bgcolor=\"#e0e0e9ff\", newrank=true];\n");

    size_t node_count = 0;
    for (const token_t* token = list->head; token; token = token->next)
    {
        node_count++;
        const size_t row = (node_count - 1) / TOKENS_PER_ROW;
        const size_t position_in_row = (node_count - 1) % TOKENS_PER_ROW;
        const size_t column = row % 2 == 0
                            ? position_in_row
                            : TOKENS_PER_ROW - position_in_row - 1;

        fprintf(txt_file, "node_%zu [shape=plain, group=\"column_%zu\", label=<<TABLE "
                          "BORDER=\"3\" CELLBORDER=\"1\" CELLSPACING=\"0\" "
                          "BGCOLOR=\"%s\" COLOR=\"#3f6969ff\">"
                          "<TR><TD COLSPAN=\"2\" WIDTH=\"%zu\">TYPE = %s</TD></TR>",
                          node_count, column, token_color(token->type),
                          TOKEN_NODE_WIDTH, token_type_to_str(token->type));

        fprintf(txt_file, "<TR><TD COLSPAN=\"2\" WIDTH=\"%zu\">", TOKEN_NODE_WIDTH);
        switch (token->type)
        {
            case OP:
                fprintf(txt_file, "OP_CODE = ");
                print_html_escaped(txt_file, operators_array[token->data_t.op].name);
                break;
            case ID:
            case INCLUDE:
                fprintf(txt_file, "ID_NUM = %d (", token->data_t.id_number);
                print_html_escaped(txt_file, identifiers[token->data_t.id_number].name);
                fprintf(txt_file, ")");
                break;
            case KEYWORD:
                fprintf(txt_file, "KEYWORD_CODE = ");
                print_html_escaped(txt_file, keywords_array[token->data_t.keyword].name);
                break;
            case NUM:
                fprintf(txt_file, "VALUE = %lg", token->data_t.number);
                break;
            case SPEC:
                fprintf(txt_file, "VALUE = %s", spec_to_str(token->data_t.spec));
                break;
            default:
                break;
        }
        fprintf(txt_file, "</TD></TR>");

        fprintf(txt_file, "<TR><TD COLSPAN=\"2\" WIDTH=\"%zu\">"
                          "line = %zu, column = %zu, length = %zu</TD></TR>"
                          "<TR><TD COLSPAN=\"2\" WIDTH=\"%zu\">ADDRESS = %p</TD></TR>"
                          "<TR><TD WIDTH=\"%zu\">next</TD><TD WIDTH=\"%zu\">%p</TD></TR>"
                          "<TR><TD WIDTH=\"%zu\">prev</TD><TD WIDTH=\"%zu\">%p</TD></TR>"
                          "</TABLE>>];\n",
                          TOKEN_NODE_WIDTH,
                          token->position.line_number, token->position.column_number,
                          token->position.length,
                          TOKEN_NODE_WIDTH, token,
                          FIELD_NAME_WIDTH, FIELD_VALUE_WIDTH, token->next,
                          FIELD_NAME_WIDTH, FIELD_VALUE_WIDTH, token->prev);
    }

    for (size_t row_start = 0; row_start < node_count; row_start += TOKENS_PER_ROW)
    {
        const size_t remaining = node_count - row_start;
        const size_t row_size = remaining < TOKENS_PER_ROW ? remaining : TOKENS_PER_ROW;
        const size_t row = row_start / TOKENS_PER_ROW;

        fprintf(txt_file, "left_rail_%zu [shape=point, width=0, height=0, "
                          "style=invis, group=\"left_rail\"];\n", row);
        fprintf(txt_file, "right_rail_%zu [shape=point, width=0, height=0, "
                          "style=invis, group=\"right_rail\"];\n", row);
        fprintf(txt_file, "{ rank=same; left_rail_%zu; ", row);

        if (row % 2 == 0)
        {
            for (size_t i = 0; i < row_size; i++)
                fprintf(txt_file, "node_%zu; ", row_start + i + 1);
        }
        else
        {
            for (size_t i = row_size; i > 0; i--)
                fprintf(txt_file, "node_%zu; ", row_start + i);
        }
        fprintf(txt_file, "right_rail_%zu; }\n", row);

        const size_t left_node = row % 2 == 0 ? row_start + 1 : row_start + row_size;
        const size_t right_node = row % 2 == 0 ? row_start + row_size : row_start + 1;
        fprintf(txt_file, "left_rail_%zu -> node_%zu [style=invis, weight=10000];\n",
                          row, left_node);
        fprintf(txt_file, "node_%zu -> right_rail_%zu [style=invis, weight=10000];\n",
                          right_node, row);

        if (row > 0)
        {
            fprintf(txt_file, "left_rail_%zu -> left_rail_%zu "
                              "[style=invis, weight=10000];\n", row - 1, row);
            fprintf(txt_file, "right_rail_%zu -> right_rail_%zu "
                              "[style=invis, weight=10000];\n", row - 1, row);
        }
    }

    fprintf(txt_file, "{\nedge[color=\"#149b5aff\", style=solid, weight=1000];\n");
    for (size_t node_number = 1; node_number < node_count; node_number++)
    {
        const size_t row = (node_number - 1) / TOKENS_PER_ROW;
        const bool changes_row = node_number % TOKENS_PER_ROW == 0;

        if (!changes_row && row % 2 != 0)
            fprintf(txt_file, "node_%zu -> node_%zu [dir=back];\n",
                              node_number + 1, node_number);
        else
            fprintf(txt_file, "node_%zu -> node_%zu;\n", node_number, node_number + 1);
    }

    fprintf(txt_file, "}\n");
    fprintf(txt_file, "}");
    fclose(txt_file);

    char command[1000];
    sprintf(command, "dot %s -T svg -o %s", txt_file_name, svg_file_name);
    system(command);

    // printf(MAKE_BOLD_GREEN("List visualization saved to %s\n"), svg_file_name);
}

static const char* token_type_to_str(type_data type)
{
    switch (type)
    {
        case OP:      return "OP";
        case ID:      return "ID";
        case KEYWORD: return "KEYWORD";
        case NUM:     return "NUM";
        case SPEC:    return "SPEC";
        case INCLUDE: return "INCLUDE";
        default:      return "UNKNOWN";
    }
}

static const char* token_color(type_data type)
{
    switch (type)
    {
        case OP:      return "#b7e5f3ff";
        case ID:      return "#36ff6fff";
        case KEYWORD: return "#f1e724ff";
        case NUM:     return "#f8c331ff";
        case SPEC:    return "#f673e9ff";
        case INCLUDE: return "#9575ffff";
        default:      return "#e0e0e0ff";
    }
}

static void print_html_escaped(FILE* file, const char* text)
{
    assert(file);
    assert(text);

    for (; *text != '\0'; text++)
    {
        switch (*text)
        {
            case '&':  fputs("&amp;",  file); break;
            case '<':  fputs("&lt;",   file); break;
            case '>':  fputs("&gt;",   file); break;
            case '"': fputs("&quot;", file); break;
            default:   fputc(*text, file);     break;
        }
    }
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
