#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "asm_dump.h"
#include "font.h"

void asm_dump(const instruction_list_t* instruction_list, const label_list_t* label_list,
              const char* const txt_file_name, const char* const png_file_name)
{
    assert(instruction_list);
    assert(label_list);
    assert(txt_file_name);
    assert(png_file_name);

    FILE* txt_file = fopen(txt_file_name, "w");
    fprintf(txt_file, "digraph structs\n{\nrankdir = LR;\ngraph[bgcolor=\"#e0e0e9ff\"];\n");

    for (size_t i = 0; i < instruction_list->count; i++)
    {
        const instruction_t* instruction = &instruction_list->instructions[i];

        fprintf(txt_file, "node_%zu [style=filled, penwidth = 3, fillcolor=\"#b7e5f3ff\","
                          "color = \"#3f6969ff\", shape=record, label= \""
                          "INDEX = %zu | MNEMONIC = %s | OPERANDS_COUNT = %zu",
                          i, i, instruction->mnemonic, instruction->operand_count);

        for (size_t j = 0; j < instruction->operand_count; j++)
        {
            const operand_t* operand = &instruction->operands[j];
            fprintf(txt_file, " | {%s", operand_kind_to_str(operand->kind));

            switch (operand->kind)
            {
                case OPERAND_REG:
                case OPERAND_XMM:
                    fprintf(txt_file, " | number = %zu", operand->reg_num);
                    break;
                case OPERAND_DOUBLE:
                    fprintf(txt_file, " | value = %lf", operand->double_value);
                    break;
                case OPERAND_IMM:
                    fprintf(txt_file, " | value = %ld", operand->imm_value);
                    break;
                case OPERAND_MEM:
                    fprintf(txt_file, " | reg = %zu, displacement = %ld", operand->reg_num, operand->displacement);
                    break;
                case OPERAND_MEM_REL:
                case OPERAND_LABEL:
                    fprintf(txt_file, " | %s", operand->label_name);
                default:
                    break;
            }

            fprintf(txt_file, "}");
        }

        fprintf(txt_file, "\"];\n");
    }

    fprintf(txt_file, "{\nedge[color=\"#149b5aff\", weight=1000];\n");
    for (size_t i = 0; i + 1 < instruction_list->count; i++)
        fprintf(txt_file, "node_%zu -> node_%zu\n", i, i + 1);
    fprintf(txt_file, "}\n");

    for (size_t i = 0; i < label_list->count; i++)
    {
        const label_t* label = &label_list->labels[i];
        fprintf(txt_file, "label_%zu [style=filled, fillcolor=\"#f8c331ff\","
                          "color=\"#3f6969ff\", shape=record, label=\"LABEL = %s | instruction_id = %zu\"];\n",
                          i, label->name, label->instruction_index);
        fprintf(txt_file, "label_%zu -> node_%zu [color=\"#f43636ff\", style=dashed];\n",
                          i, label->instruction_index);
    }

    fprintf(txt_file, "}");
    fclose(txt_file);

    char command[1000];
    sprintf(command, "dot %s -T png -o %s", txt_file_name, png_file_name);
    system(command);

    printf(MAKE_BOLD_GREEN("ASM dump saved to %s\n"), png_file_name);
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
