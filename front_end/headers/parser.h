#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#include "tokenization.h"

error_code tokens_to_tree(list_t* list, node_t** node_ptr);

#endif // PARSER_H
