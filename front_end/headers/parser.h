#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#include "tokenization.h"

error_code tokens_to_tree(list_t* list, node_t** node_ptr);
node_t* GetG(token_t** token);
node_t* GetP(token_t** token);
node_t* GetE(token_t** token);
node_t* GetT(token_t** token);
node_t* GetS(token_t** token);
node_t* GetF(token_t** token);
node_t* GetN(token_t** token);
node_t* GetV(token_t** token);

#endif // PARSER_H
