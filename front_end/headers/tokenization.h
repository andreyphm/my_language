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
    spec_code spec;
};

struct position_t
{
    size_t line_number;
    size_t column_number;
    size_t length;
};

struct token_t
{
    type_data type;
    token_union data_t;
    position_t position; 
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
void skip_spaces(const char** string, position_t* const position);

bool try_digit(const char** buffer, list_t* const list, position_t* const position);
bool try_op(const char** buffer, list_t* const list, position_t* const position);
bool try_keyword(const char** buffer, list_t* const list, position_t* const position);
bool try_spec(const char** buffer, list_t* const list, position_t* const position);
bool try_identifier(const char** buffer, list_t* const list, position_t* const position,
                    identifier_t* identifiers, int* last_identifier_num, bool* is_identifiers);

bool is_char(const char symbol);

token_t* list_push_back(const type_data type, token_union data, const position_t* const position, list_t* const list);
token_t* create_token(const type_data type, token_union data, const position_t* const position);
void list_destroy(list_t* list);

#endif //TOKENIZATION_H
