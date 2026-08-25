#ifndef INSTRUCTIONS_DUMP_H
#define INSTRUCTIONS_DUMP_H

#include "instructions.h"

#define INSTRUCTIONS_DUMP_TXT "back_end/source/instructions_dump/instructions_dump.txt"
#define INSTRUCTIONS_DUMP_SVG "back_end/source/instructions_dump/instructions_dump.svg"

void instructions_dump(const instruction_list_t* instruction_list, const label_list_t* label_list,
                       const char* const txt_file_name, const char* const svg_file_name);
const char* operand_kind_to_str(operand_kind kind);

#endif // INSTRUCTIONS_DUMP_H
