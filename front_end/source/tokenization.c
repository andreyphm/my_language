#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "input.h"
#include "tokenization.h"
#include "font.h"
#include "dump.h"
#include "output.h"

error_code file_to_tokens(variable_t** variables_ptr, FILE* input_file, list_t* list)
{
    char* buffer = nullptr;
    char* original_ptr = nullptr;

    *variables_ptr = (variable_t*) calloc(MAX_NUMBER_OF_VARS, sizeof(variable_t));
    
    // rewind(input_file);
    buffer = read_file_to_buffer(input_file);
    size_t buffer_len = strlen(buffer);
    buffer[buffer_len] = '$';
    original_ptr = buffer;
                    
    list->head = create_token(SPEC, (token_union){.spec_symbol = '!'});
    list->tail = list->head;

    error_code error = tokenization(buffer, *variables_ptr, list);
    if (error)
    {
        free(original_ptr);
        list_destroy(list);
        error_message(error);
        return error;
    }

    free(original_ptr);
    printf(MAKE_BOLD_GREEN("Successfully\n"));
    return NO_ERROR;
}

error_code tokenization(const char* buffer, variable_t* variables, list_t* const list)
{
    bool is_variables = false;
    int last_variable_num = 0;

    while (*buffer != '$')
    {
        skip_spaces(&buffer);
        if (*buffer == '$') break;

        if (try_digit(&buffer, list)            ||
            try_char_op(&buffer, list)          ||
            try_arithm_function(&buffer, list)  ||
            try_bracket(&buffer, list)          ||
            try_assign_op(&buffer, list)        ||
            try_variable(&buffer, list, variables, &last_variable_num, &is_variables))
        {
            continue;
        }

        else return SYNTAX_ERROR;
    }

    list_push_back(SPEC, (token_union){.spec_symbol = '$'}, list);

    token_t* new_head = list->head->next;
    free(list->head);
    list->head = new_head;

    list_dump(list, LIST_DUMP_TXT, LIST_DUMP_PNG, variables);

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

bool try_char_op(const char** buffer, list_t* const list)
{
    for (size_t i = 0; i <= LAST_CHAR_OP_NUM; i++)
    {
        if (!strncmp(operators_array[i].design, *buffer, 1))
        {
            list_push_back(OP, (token_union){.op = operators_array[i].code}, list);
            (*buffer)++;

            return true;
        }
    }

    return false;
}

bool try_arithm_function(const char** buffer, list_t* const list)
{
    const char* start_of_buffer = *buffer;

    if (isalpha(**buffer))
    {
        (*buffer)++;

        while (isalpha(**buffer))
            (*buffer)++;

        for (size_t i = FIRST_FUNC_NUM; i <= LAST_FUNC_NUM; i++)
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

bool try_bracket(const char** buffer, list_t* const list)
{
    if (**buffer == '(' || **buffer == ')')
    {
        list_push_back(SPEC, (token_union){.spec_symbol = **buffer}, list);
        (*buffer)++;
        return true;
    }

    return false;
}

bool try_assign_op(const char** buffer, list_t* const list)
{
    if (**buffer == '=')
    {
        list_push_back(OP, (token_union){.op = ASSIGN}, list);
        (*buffer)++;
        return true;
    }

    return false;
}

bool try_variable(const char** buffer, list_t* const list, variable_t* variables, int* last_variable_num, bool* is_variables)
{
    const char* start_of_buffer = *buffer;

    if (!isalpha(**buffer))
        return false;

    while (isalpha(**buffer))
        (*buffer)++;

    size_t name_length = (size_t) (*buffer - start_of_buffer);

    if (*is_variables)
    {
        for (int i = 0; i <= *last_variable_num; i++)
        {
            if (variables[i].length == name_length && !strncmp(variables[i].name, start_of_buffer, name_length))
            {
                list_push_back(VAR, (token_union){.var_number = i}, list);
                return true;
            }
        }
    }

    if (!*is_variables)
    {
        list_push_back(VAR, (token_union){.var_number = 0}, list);
        *is_variables = true;
    }

    else
    {
        (*last_variable_num)++;
        list_push_back(VAR, (token_union){.var_number = *last_variable_num}, list);
    }

    variables[*last_variable_num].name = strndup(start_of_buffer, name_length);
    variables[*last_variable_num].number = *last_variable_num;
    variables[*last_variable_num].length = name_length;

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

        case VAR:
            token->data_t.var_number = data.var_number;
            break;

        case NUM:
            token->data_t.number = data.number;
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

void variables_destroy(variable_t** variables)
{
    for (size_t i = 0; i < MAX_NUMBER_OF_VARS; i++)
            free((void*)(*variables)[i].name);

    free(*variables);
    *variables = nullptr;
}
