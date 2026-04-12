#ifndef TOKENIZATION_H
#define TOKENIZATION_H

#include "front_end.h"

const int MAX_NUMBER_OF_IDENTIFIERS = 30;

union token_union
{
    double number;
    int id_number;
    operator_code op;
    keyword_code keyword;
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

error_code file_to_tokens(identifier_t** identifiers_ptr, FILE* input_file, list_t* list);
error_code tokenization(const char* buffer, identifier_t* identifiers, list_t* const list);
void skip_spaces(const char** string);

bool try_digit(const char** buffer, list_t* const list);
bool try_op(const char** buffer, list_t* const list);
bool try_keyword(const char** buffer, list_t* const list);
bool try_spec_symbol(const char** buffer, list_t* const list);
bool try_identifier(const char** buffer, list_t* const list, identifier_t* identifiers,
                                                int* last_function_num, bool* is_functions);

void init_operations_mask(unsigned char* array);
void init_spec_symbols_mask(unsigned char* array);

token_t* list_push_back(const type_data type, token_union data, list_t* const list);
token_t* create_token(const type_data type, token_union data);
void list_destroy(list_t* list);

#endif //TOKENIZATION_H
