#ifndef DUMP_H
#define DUMP_H

#include "tokenization.h"

#define TREE_DUMP_TXT "source/tree_dump/tree_dump.txt"
#define TREE_DUMP_PNG "source/tree_dump/tree_dump.png"
#define LIST_DUMP_TXT "source/list_dump/list_dump.txt"
#define LIST_DUMP_PNG "source/list_dump/list_dump.png"

void tree_dump(node_t* const node, const char* const png_file_name, const identifier_t* const identifiers);
void list_dump(list_t* const list, const char* const txt_file_name, const char* const png_file_name,
                                                                const identifier_t* const identifiers);

const char* spec_to_str(char symbol);
const char* node_kind_to_str(node_kind kind);

#endif // DUMP_H
