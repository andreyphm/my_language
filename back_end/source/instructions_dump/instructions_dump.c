#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>

#include "instructions_dump.h"
#include "font.h"

static const size_t INSTRUCTIONS_PER_ROW    = 20;
static const size_t INSTRUCTION_NODE_WIDTH  = 260;
static const size_t OPERAND_KIND_WIDTH      = 80;
static const size_t OPERAND_VALUE_WIDTH     = INSTRUCTION_NODE_WIDTH - OPERAND_KIND_WIDTH;

static void print_html_escaped(FILE* file, const char* text);

void instructions_dump(const instruction_list_t* instruction_list, const label_list_t* label_list,
                       const char* const txt_file_name, const char* const svg_file_name)
{
    assert(instruction_list);
    assert(label_list);
    assert(txt_file_name);
    assert(svg_file_name);

    FILE* txt_file = fopen(txt_file_name, "w");
    fprintf(txt_file, "digraph structs\n{\nrankdir = TB;\n"
                      "graph[bgcolor=\"#e0e0e9ff\", newrank=true];\n");

    for (size_t i = 0; i < instruction_list->count; i++)
    {
        const instruction_t* instruction = &instruction_list->instructions[i];
        const size_t row = i / INSTRUCTIONS_PER_ROW;
        const size_t position_in_row = i % INSTRUCTIONS_PER_ROW;
        const size_t column = row % 2 == 0
                            ? position_in_row
                            : INSTRUCTIONS_PER_ROW - position_in_row - 1;

        fprintf(txt_file, "node_%zu [shape=plain, group=\"column_%zu\", label=<<TABLE "
                          "BORDER=\"3\" CELLBORDER=\"1\" CELLSPACING=\"0\" "
                          "BGCOLOR=\"#b7e5f3ff\" COLOR=\"#3f6969ff\">",
                          i, column);

        for (size_t label_index = 0; label_index < label_list->count; label_index++)
        {
            const label_t* label = &label_list->labels[label_index];
            if (label->instruction_index != i)
                continue;

            fprintf(txt_file, "<TR><TD COLSPAN=\"2\" BGCOLOR=\"#f8c331ff\" "
                              "WIDTH=\"%zu\">LABEL = ", INSTRUCTION_NODE_WIDTH);
            print_html_escaped(txt_file, label->name);
            fprintf(txt_file, "</TD></TR>");
        }

        fprintf(txt_file, "<TR><TD COLSPAN=\"2\" WIDTH=\"%zu\">INDEX = %zu</TD></TR>"
                          "<TR><TD COLSPAN=\"2\" WIDTH=\"%zu\">MNEMONIC = ",
                          INSTRUCTION_NODE_WIDTH, i, INSTRUCTION_NODE_WIDTH);
        print_html_escaped(txt_file, instruction->mnemonic);
        fprintf(txt_file, "</TD></TR>"
                          "<TR><TD COLSPAN=\"2\" WIDTH=\"%zu\">"
                          "OPERANDS_COUNT = %zu</TD></TR>",
                          INSTRUCTION_NODE_WIDTH, instruction->operand_count);

        for (size_t j = 0; j < instruction->operand_count; j++)
        {
            const operand_t* operand = &instruction->operands[j];
            fprintf(txt_file, "<TR><TD WIDTH=\"%zu\">%s</TD><TD WIDTH=\"%zu\">",
                    OPERAND_KIND_WIDTH, operand_kind_to_str(operand->kind),
                    OPERAND_VALUE_WIDTH);

            switch (operand->kind)
            {
                case OPERAND_REG:
                case OPERAND_XMM:
                    fprintf(txt_file, "number = %zu", operand->reg_num);
                    break;
                case OPERAND_DOUBLE:
                    fprintf(txt_file, "value = %lf", operand->double_value);
                    break;
                case OPERAND_IMM:
                    fprintf(txt_file, "value = %" PRId64, operand->imm_value);
                    break;
                case OPERAND_MEM:
                    fprintf(txt_file, "reg = %zu, displacement = %" PRId64,
                            operand->reg_num, operand->displacement);
                    break;
                case OPERAND_MEM_REL:
                case OPERAND_LABEL:
                    print_html_escaped(txt_file, operand->label_name);
                    break;
                default:
                    break;
            }

            fprintf(txt_file, "</TD></TR>");
        }

        fprintf(txt_file, "</TABLE>>];\n");
    }

    for (size_t row_start = 0; row_start < instruction_list->count;
         row_start += INSTRUCTIONS_PER_ROW)
    {
        const size_t remaining = instruction_list->count - row_start;
        const size_t row_size = remaining < INSTRUCTIONS_PER_ROW
                              ? remaining
                              : INSTRUCTIONS_PER_ROW;
        const size_t row = row_start / INSTRUCTIONS_PER_ROW;

        fprintf(txt_file, "left_rail_%zu [shape=point, width=0, height=0, "
                          "style=invis, group=\"left_rail\"];\n", row);
        fprintf(txt_file, "right_rail_%zu [shape=point, width=0, height=0, "
                          "style=invis, group=\"right_rail\"];\n", row);

        fprintf(txt_file, "{ rank=same; left_rail_%zu; ", row);
        if (row % 2 == 0)
        {
            for (size_t i = 0; i < row_size; i++)
                fprintf(txt_file, "node_%zu; ", row_start + i);
        }
        else
        {
            for (size_t i = row_size; i > 0; i--)
                fprintf(txt_file, "node_%zu; ", row_start + i - 1);
        }
        fprintf(txt_file, "right_rail_%zu; }\n", row);

        const size_t left_node = row % 2 == 0
                               ? row_start
                               : row_start + row_size - 1;
        const size_t right_node = row % 2 == 0
                                ? row_start + row_size - 1
                                : row_start;

        fprintf(txt_file, "left_rail_%zu -> node_%zu "
                          "[style=invis, weight=10000];\n", row, left_node);
        fprintf(txt_file, "node_%zu -> right_rail_%zu "
                          "[style=invis, weight=10000];\n", right_node, row);

        if (row > 0)
        {
            fprintf(txt_file, "left_rail_%zu -> left_rail_%zu "
                              "[style=invis, weight=10000];\n", row - 1, row);
            fprintf(txt_file, "right_rail_%zu -> right_rail_%zu "
                              "[style=invis, weight=10000];\n", row - 1, row);
        }
    }

    fprintf(txt_file, "{\nedge[color=\"#149b5aff\", style=solid, weight=1000];\n");
    for (size_t i = 0; i + 1 < instruction_list->count; i++)
    {
        const size_t row = i / INSTRUCTIONS_PER_ROW;
        const bool changes_row = (i + 1) % INSTRUCTIONS_PER_ROW == 0;

        if (!changes_row && row % 2 != 0)
            fprintf(txt_file, "node_%zu -> node_%zu [dir=back];\n", i + 1, i);
        else
            fprintf(txt_file, "node_%zu -> node_%zu;\n", i, i + 1);
    }
    fprintf(txt_file, "}\n");

    fprintf(txt_file, "}");
    fclose(txt_file);

    char command[1000];
    sprintf(command, "dot %s -T svg -o %s", txt_file_name, svg_file_name);
    system(command);

    // printf(MAKE_BOLD_GREEN("Instructions dump saved to %s\n"), svg_file_name);
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

const char* operand_kind_to_str(operand_kind kind)
{
    switch (kind)
    {
        case NO_OPERAND:        return "NO_OPERAND";
        case OPERAND_REG:       return "REG";
        case OPERAND_XMM:       return "XMM";
        case OPERAND_MEM:       return "MEM";
        case OPERAND_MEM_REL:   return "MEM_REL";
        case OPERAND_DOUBLE:    return "DOUBLE";
        case OPERAND_IMM:       return "IMM";
        case OPERAND_LABEL:     return "LABEL";

        default:                return "UNKNOWN";
    }
}
