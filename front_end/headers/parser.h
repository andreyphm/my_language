#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#include "tokenization.h"

error_code tokens_to_tree(list_t* list, node_t** node_ptr);
node_t* get_prog(token_t** token);
node_t* get_func(token_t** token);
node_t* get_params(token_t** token);
node_t* get_block(token_t** token);
node_t* get_op(token_t** token);
node_t* get_if(token_t** token);
node_t* get_else(token_t** token);
node_t* get_while(token_t** token);
node_t* get_var_declare(token_t** token);
node_t* get_ret(token_t** token);
node_t* get_break(token_t** token);
node_t* get_e(token_t** token);
node_t* get_or(token_t** token);
node_t* get_and(token_t** token);
node_t* get_bit_or(token_t** token);
node_t* get_bit_xor(token_t** token);
node_t* get_bit_and(token_t** token);
node_t* get_equal_or_not(token_t** token);
node_t* get_less_greater(token_t** token);
node_t* get_shift(token_t** token);
node_t* get_add_sub(token_t** token);
node_t* get_mul_div(token_t** token);
node_t* get_p(token_t** token);
node_t* get_call(token_t** token);
node_t* get_args(token_t** token);
node_t* get_n(token_t** token);
node_t* get_id(token_t** token);
bool token_is_start_of_expr(token_t* const* const token);

#endif // PARSER_H
