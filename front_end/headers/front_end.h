#ifndef FRONT_END_H
#define FRONT_END_H

#include <stdbool.h>
#include <assert.h>

#include "tree.h"

const size_t FIRST_CHILDREN_NUMBER  = 3;

enum error_code
{
    NO_ERROR        = 0,
    SYNTAX_ERROR    = 1,
    TREE_NULLPTR    = 2
};

struct operator_t
{
    operator_code code;
    const char* name;
    const char* design;
    size_t strlen;
};

struct keyword_t
{
    keyword_code code;
    const char* name;
    const char* design;
    size_t strlen;
};

void identifiers_destroy(identifier_t** identifiers);

const operator_t operators_array[] =
{
    {ADD,           "ADD",              "+",    1},
    {SUB,           "SUB",              "-",    1},
    {MUL,           "MUL",              "*",    1},
    {DIV,           "DIV",              "/",    1},
    {IS_EQUAL,      "IS_EQUAL",         "==",   2},
    {ASSIGN,        "ASSIGN",           "=",    1},
    {LOGIC_OR,      "LOGIC_OR",         "||",   2},
    {LOGIC_AND,     "LOGIC_AND",        "&&",   2},
    {IS_NOT_EQUAL,  "IS_NOT_EQUAL",     "!=",   2},
    {SHR,           "SHR",              ">>",   2},
    {GREATER_EQUAL, "GREATER_EQUAL",    ">=",   2},
    {GREATER,       "GREATER",          ">",    1},
    {SHL,           "SHL",              "<<",   2},
    {LESS_EQUAL,    "GREATER",          ">=",   2},
    {LESS,          "LESS",             "<",    1},
    {BIT_OR,        "BIT_OR",           "|",    1},
    {BIT_XOR,       "BIT_XOR",          "^",    1},
    {BIT_AND,       "BIT_AND",          "&",    1}
};

const size_t OP_ARRAY_SIZE = sizeof(operators_array)/sizeof(operators_array[0]);

const keyword_t keywords_array[] =
{
    {IF,        "IF",       "if",       2},
    {ELSE,      "ELSE",     "else",     4},
    {WHILE,     "WHILE",    "while",    5},
    {VAR_DECL,  "VAR",      "var",      3},
    {RET,       "RET",      "return",   6},
    {BREAK,     "BREAK",    "break",    5},
    {CALL,      "CALL",     "call",     4},
    {FUNC,      "FUNC",     "func",     4}
};

const size_t KEYWORD_ARRAY_SIZE = sizeof(keywords_array)/sizeof(keywords_array[0]);

node_t* front_end_run(FILE* input_file, identifier_t** identifiers);

#endif // FRONT_END_H
