#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "input.h"
#include "tokenization.h"
#include "font.h"
#include "output.h"

error_code file_to_tokens(identifier_t** identifiers_ptr, FILE* input_file, list_t* list)
{
    char* buffer = nullptr;
    char* original_ptr = nullptr;

    *identifiers_ptr = (identifier_t*) calloc(MAX_NUMBER_OF_IDENTIFIERS, sizeof(identifier_t));
    
    buffer = read_file_to_buffer(input_file);
    size_t buffer_len = strlen(buffer);
    buffer[buffer_len] = '$';
    original_ptr = buffer;
                    
    list->head = create_token(SPEC, (token_union){.spec_symbol = '!'});
    list->tail = list->head;

    error_code error = tokenization(buffer, *identifiers_ptr, list);
    if (error)
    {
        free(original_ptr);
        list_destroy(list);
        error_message(error);
        return error;
    }

    free(original_ptr);
    printf(MAKE_BOLD_GREEN("Tokenization successful\n"));
    return NO_ERROR;
}

error_code tokenization(const char* buffer, identifier_t* identifiers, list_t* const list)
{
    bool is_identifiers = false;
    int last_identifier_num = 0;

    while (*buffer != '$')
    {
        skip_spaces(&buffer);
        if (*buffer == '$') break;

        if (try_digit(&buffer, list)        ||
            try_op(&buffer, list)           ||
            try_spec_symbol(&buffer, list)  ||
            try_keyword(&buffer, list)      ||
            try_identifier(&buffer, list, identifiers, &last_identifier_num, &is_identifiers))
        {
            continue;
        }

        else return SYNTAX_ERROR;
    }

    list_push_back(SPEC, (token_union){.spec_symbol = '$'}, list);

    token_t* new_head = list->head->next;
    free(list->head);
    list->head = new_head;

    return NO_ERROR;
}

void skip_spaces(const char** string)
{
    while (isspace(**string))
        (*string)++;
}

bool try_digit(const char** buffer, list_t* const list)
{
    bool dot_already = false;
    double value = 0;

    if (isdigit(**buffer))
    {
        sscanf(*buffer, "%lf", &value);
        list_push_back(NUM, (token_union){.number = value}, list);

        while (isdigit(**buffer))
        {
            (*buffer)++;
            if (**buffer == '.' && !dot_already)
            {
                (*buffer)++;
                dot_already = true;
            }
        }

        return true;
    }

    return false;
}

bool try_op(const char** buffer, list_t* const list)
{
    static unsigned char is_op_symbol[256] = {};
    static bool array_filled = false;
    if (!array_filled)  
    {
        init_operations_mask(is_op_symbol);
        array_filled = true;
    }
    const char* start_of_buffer = *buffer;
    
    if (is_op_symbol[(unsigned char)**buffer])
    {
        (*buffer)++;

        while (is_op_symbol[(unsigned char)**buffer])
            (*buffer)++;

        for (size_t i = 0; i < OP_ARRAY_SIZE; i++)
        {
            if (!strncmp(operators_array[i].design, start_of_buffer, (size_t) operators_array[i].strlen))
            {
                list_push_back(OP, (token_union){.op = operators_array[i].code}, list);
                return true;
            }
        }

        *buffer = start_of_buffer;
        return false;
    }

    return false;
}

bool try_keyword(const char** buffer, list_t* const list)
{
    const char* start_of_buffer = *buffer;

    if (isalpha(**buffer))
    {
        (*buffer)++;

        while (isalpha(**buffer))
            (*buffer)++;

        size_t word_length = (size_t)(*buffer - start_of_buffer);
        for (size_t i = 0; i < KEYWORD_ARRAY_SIZE; i++)
        {
            if (word_length == (size_t)keywords_array[i].strlen &&
                !strncmp(keywords_array[i].design, start_of_buffer, (size_t) keywords_array[i].strlen))
            {
                list_push_back(KEYWORD, (token_union){.keyword = keywords_array[i].code}, list);
                return true;
            }
        }

        *buffer = start_of_buffer;
        return false;
    }

    return false;
}

bool try_spec_symbol(const char** buffer, list_t* const list)
{
    static unsigned char is_spec_symbol[256] = {};
    static bool array_filled = false;
    if (!array_filled)  
    {
        init_spec_symbols_mask(is_spec_symbol);
        array_filled = true;
    }

    if (is_spec_symbol[(unsigned char)**buffer])
    {
        list_push_back(SPEC, (token_union){.spec_symbol = **buffer}, list);
        (*buffer)++;
        return true;
    }

    return false;
}

bool try_identifier(const char** buffer, list_t* const list, identifier_t* identifiers, 
                                            int* last_identifier_num, bool* is_identifiers)
{
    const char* start_of_buffer = *buffer;

    if (!(isalpha(**buffer) || **buffer == '_'))
        return false;

    (*buffer)++;

    while (isalpha(**buffer) || **buffer == '_' || isdigit(**buffer))
        (*buffer)++;

    size_t name_length = (size_t) (*buffer - start_of_buffer);

    if (*is_identifiers)
    {
        for (int i = 0; i <= *last_identifier_num; i++)
        {
            if (identifiers[i].length == name_length && !strncmp(identifiers[i].name, start_of_buffer, name_length))
            {
                list_push_back(ID, (token_union){.id_number = i}, list);
                return true;
            }
        }
    }

    if (!*is_identifiers)
    {
        list_push_back(ID, (token_union){.id_number = 0}, list);
        *is_identifiers = true;
    }

    else
    {
        (*last_identifier_num)++;
        list_push_back(ID, (token_union){.id_number = *last_identifier_num}, list);
    }

    identifiers[*last_identifier_num].name = strndup(start_of_buffer, name_length);
    identifiers[*last_identifier_num].number = *last_identifier_num;
    identifiers[*last_identifier_num].length = name_length;

    return true;
}

token_t* list_push_back(const type_data type, token_union data, list_t* const list)
{
    assert(list);

    token_t* const token = create_token(type, data);

    token->prev = list->tail;
    list->tail->next = token;
    list->tail = token;

    return token;
}

token_t* create_token(const type_data type, token_union data)
{
    token_t* const token = (token_t*) calloc(1, sizeof(token_t));

    token->type = type;

    switch(type)
    {
        case OP:
            token->data_t.op = data.op;
            break;

        case ID:
            token->data_t.id_number = data.id_number;
            break;

        case NUM:
            token->data_t.number = data.number;
            break;

        case KEYWORD:
            token->data_t.keyword = data.keyword;
            break;

        case SPEC:
            token->data_t.spec_symbol = data.spec_symbol;

        default:
            break;
    }

    token->prev = nullptr;
    token->next = nullptr;

    return token;
}

void list_destroy(list_t* list)
{
    token_t* current = list->head;

    while (current)
    {
        token_t* next = current->next;
        free(current);
        current = next;
    }
}

void identifiers_destroy(identifier_t** identifiers)
{
    for (size_t i = 0; i < MAX_NUMBER_OF_IDENTIFIERS; i++)
            free((void*)(*identifiers)[i].name);

    free(*identifiers);
    *identifiers = nullptr;
}

void init_operations_mask(unsigned char* array)
{
    array[(unsigned char)'+'] = 1;
    array[(unsigned char)'-'] = 1;
    array[(unsigned char)'*'] = 1;
    array[(unsigned char)'/'] = 1;
    array[(unsigned char)'<'] = 1;
    array[(unsigned char)'>'] = 1;
    array[(unsigned char)'='] = 1;
    array[(unsigned char)'&'] = 1;
    array[(unsigned char)'|'] = 1;
    array[(unsigned char)'^'] = 1;
    array[(unsigned char)'!'] = 1;
}

void init_spec_symbols_mask(unsigned char* array)
{
    array[(unsigned char)'('] = 1;
    array[(unsigned char)')'] = 1;
    array[(unsigned char)'{'] = 1;
    array[(unsigned char)'}'] = 1;
    array[(unsigned char)';'] = 1;
    array[(unsigned char)','] = 1;
}
