#ifndef FRONT_END_H
#define FRONT_END_H

#include <stdbool.h>
#include <assert.h>

#include "tree.h"

enum error_code
{
    NO_ERROR = 0,
    SYNTAX_ERROR,
    TREE_NULLPTR,
    MULTIPLE_VAR_DECLARATION,
    PROG_NODE_ERROR,
    PROG_CHILD_COUNT_ERROR,
    UNDECLARED_VARIABLE,
    UNDECLARED_FUNCTION,
    MULTIPLE_FUNC_DECLARATION,
    FUNC_WRONG_NUMBER_OF_ARGS,
    BREAK_OUTSIDE_LOOP,
    MISSING_ENTRY_POINT,
    MULTIPLE_ENTRY_POINTS,
    OUT_WRONG_NUMBER_OF_ARGS,
    IN_WRONG_NUMBER_OF_ARGS,
    SQRT_WRONG_NUMBER_OF_ARGS
};

struct token_info_t
{
    int code;
    const char* name;
    const char* design;
    size_t strlen;
};

void identifiers_destroy(identifier_t** identifiers);

extern const token_info_t operators_array[];
extern const size_t OP_ARRAY_SIZE;
extern const token_info_t keywords_array[];
extern const size_t KEYWORD_ARRAY_SIZE;
extern const token_info_t specs_array[];
extern const size_t SPEC_ARRAY_SIZE;

node_t* front_end_run(FILE* input_file, identifier_t** identifiers);

#endif // FRONT_END_H
