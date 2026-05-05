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

    error_code error = tokenization(buffer, *identifiers_ptr, list);
    if (error)
    {
        free(original_ptr);
        list_destroy(list);
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

    lexer_state_t position =
    {
        .line_number = 1,
        .column_number = 1
    };

    while (*buffer != '$')
    {
        skip_spaces(&buffer, &position);
        if (*buffer == '$') break;

        const char* start_of_buffer = buffer;

        if (try_digit(&buffer, list, position)   ||
            try_op(&buffer, list, position)      ||
            try_spec(&buffer, list, position)    ||
            try_keyword(&buffer, list, position) ||
            try_identifier(&buffer, list, position, identifiers, &last_identifier_num, &is_identifiers))
        {
            position.column_number += (size_t)(buffer - start_of_buffer);
            continue;
        }

        else return SYNTAX_ERROR;
    }

    list_push_back(SPEC, (token_union){.spec = PROGRAM_END}, position, list);

    return NO_ERROR;
}

void skip_spaces(const char** string, lexer_state_t* const position)
{
    assert(position);

    while (isspace(**string))
    {
        if (**string == '\n')
        {
            (position->column_number) = 1;
            (position->line_number)++;
        }

        else
            (position->column_number)++;

        (*string)++;
    }
}

bool is_char(const char symbol)
{
    return (isalpha((unsigned char)symbol) ||
            isdigit((unsigned char)symbol) ||
            symbol == '_');
}

bool try_digit(const char** buffer, list_t* const list, const lexer_state_t position)
{
    const char* start_of_buffer = *buffer;
    bool dot_already = false;
    double value = 0;

    if (isdigit(**buffer))
    {
        sscanf(*buffer, "%lf", &value);

        while (isdigit(**buffer))
        {
            (*buffer)++;

            if (**buffer == '.' && !dot_already)
            {
                (*buffer)++;
                dot_already = true;
            }
        }

        if (isalpha((unsigned char)**buffer) || **buffer == '_')
        {
            *buffer = start_of_buffer;
            return false;
        }

        list_push_back(NUM, (token_union){.number = value}, position, list);
        return true;
    }

    return false;
}

bool try_op(const char** buffer, list_t* const list, const lexer_state_t position)
{
    size_t index = OP_ARRAY_SIZE;
    size_t length = 0;

    for (size_t i = 0; i < OP_ARRAY_SIZE; i++)
    {
        if (!strncmp(*buffer, operators_array[i].design, operators_array[i].strlen) &&
            operators_array[i].strlen > length)
        {
            index = i;
            length = operators_array[i].strlen;
        }
    }

    if (index == OP_ARRAY_SIZE)
        return false;

    list_push_back(OP, (token_union){.op = (operator_code)operators_array[index].code}, position, list);
    *buffer += length;
    return true;
}

bool try_keyword(const char** buffer, list_t* const list, const lexer_state_t position)
{
    size_t index = KEYWORD_ARRAY_SIZE;
    size_t length = 0;

    for (size_t i = 0; i < KEYWORD_ARRAY_SIZE; i++)
    {
        if (!strncmp(*buffer, keywords_array[i].design, keywords_array[i].strlen) &&
            keywords_array[i].strlen > length)
        {
            index = i;
            length = keywords_array[i].strlen;
        }
    }

    if (index == KEYWORD_ARRAY_SIZE)
        return false;

    list_push_back(KEYWORD, (token_union){.keyword = (keyword_code)keywords_array[index].code}, position, list);
    *buffer += length;
    return true;
}

bool try_spec(const char** buffer, list_t* const list, const lexer_state_t position)
{
    size_t index = SPEC_ARRAY_SIZE;
    size_t length = 0;

    for (size_t i = 0; i < SPEC_ARRAY_SIZE; i++)
    {
        if (!strncmp(*buffer, specs_array[i].design, specs_array[i].strlen) &&
            specs_array[i].strlen > length)
        {
            index = i;
            length = specs_array[i].strlen;
        }
    }

    if (index == SPEC_ARRAY_SIZE)
        return false;

    list_push_back(SPEC, (token_union){.spec = (spec_code)specs_array[index].code}, position, list);
    *buffer += length;
    return true;
}

bool try_identifier(const char** buffer, list_t* const list, const lexer_state_t position,
                    identifier_t* identifiers, int* last_identifier_num, bool* is_identifiers)
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
                list_push_back(ID, (token_union){.id_number = i}, position, list);
                return true;
            }
        }
    }

    if (!*is_identifiers)
    {
        list_push_back(ID, (token_union){.id_number = 0}, position, list);
        *is_identifiers = true;
    }

    else
    {
        (*last_identifier_num)++;
        list_push_back(ID, (token_union){.id_number = *last_identifier_num}, position, list);
    }

    identifiers[*last_identifier_num].name = strndup(start_of_buffer, name_length);
    identifiers[*last_identifier_num].number = *last_identifier_num;
    identifiers[*last_identifier_num].length = name_length;

    return true;
}

token_t* list_push_back(const type_data type, token_union data, const lexer_state_t position, list_t* const list)
{
    assert(list);

    token_t* const token = create_token(type, data, position);

    if (!list->head && !list->current && !list->tail)
    {
        list->head = token;
        list->tail = token;
        list->current = token;
    }

    list->tail->next = token;
    list->tail = token;

    return token;
}

token_t* create_token(const type_data type, token_union data, const lexer_state_t position)
{
    token_t* const token = (token_t*) calloc(1, sizeof(token_t));

    token->type = type;
    token->position = {};

    token->position.column_number = position.column_number;
    token->position.line_number = position.line_number;

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
            token->data_t.spec = data.spec;

        default:
            break;
    }

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
