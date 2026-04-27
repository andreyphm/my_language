#ifndef DUMP_H
#define DUMP_H

#include "tokenization.h"

#define LIST_DUMP_TXT "front_end/source/list_dump/list_dump.txt"
#define LIST_DUMP_PNG "front_end/source/list_dump/list_dump.png"

void list_dump(list_t* const list, const char* const txt_file_name, const char* const png_file_name,
               const identifier_t* const identifiers);

const char* spec_to_str(spec_code spec);
const char* node_kind_to_str(node_kind kind);

#endif // DUMP_H
