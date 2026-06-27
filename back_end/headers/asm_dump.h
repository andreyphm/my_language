#ifndef ASM_DUMP_H
#define ASM_DUMP_H

#include "asm_to_binary.h"

#define ASM_DUMP_TXT "back_end/source/asm_dump/asm_dump.txt"
#define ASM_DUMP_PNG "back_end/source/asm_dump/asm_dump.png"

void asm_dump(const instruction_list_t* instruction_list, const label_list_t* label_list,
              const char* const txt_file_name, const char* const png_file_name);
const char* operand_kind_to_str(operand_kind kind);

#endif // ASM_DUMP_H
