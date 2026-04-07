#ifndef TOKENIZATION_H
#define TOKENIZATION_H

#include "front_end.h"

const int MAX_NUMBER_OF_VARS = 20;

union token_union
{
    double number;
    int var_number;
    operator_code op;
    char spec_symbol;
};

struct token_t
{
    type_data type;
    token_union data_t;
    token_t* prev;
    token_t* next;
};

struct list_t
{
    token_t* head;
    token_t* current;
    token_t* tail;
};

error_code file_to_tokens(variable_t** variables_ptr, FILE* input_file, list_t* list);
error_code tokenization(const char* buffer, variable_t* variables, list_t* const list);
void skip_spaces(const char** string);

bool try_digit(const char** buffer, list_t* const list);
bool try_char_op(const char** buffer, list_t* const list);
bool try_arithm_function(const char** buffer, list_t* const list);
bool try_bracket(const char** buffer, list_t* const list);
bool try_assign_op(const char** buffer, list_t* const list);
bool try_variable(const char** buffer, list_t* const list, variable_t* variables, int* last_variable_num, bool* is_variables);

token_t* list_push_back(const type_data type, token_union data, list_t* const list);
token_t* create_token(const type_data type, token_union data);
void list_destroy(list_t* list);

#endif //TOKENIZATION_H
