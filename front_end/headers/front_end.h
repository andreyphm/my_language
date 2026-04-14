#ifndef FRONT_END_H
#define FRONT_END_H

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

const double NUMBER_CLOSE_TO_ZERO   = 10e-12;
const size_t FIRST_CHILDREN_NUMBER  = 16;

enum operator_code
{
    ADD             = 0,
    SUB             = 1,
    MUL             = 2,
    DIV             = 3,
    IS_EQUAL        = 4,
    ASSIGN          = 5,
    LOGIC_OR        = 6,
    LOGIC_AND       = 7,
    IS_NOT_EQUAL    = 8,
    SHR             = 9,
    GREATER_EQUAL   = 10,
    GREATER         = 11,
    SHL             = 12,
    LESS_EQUAL      = 13,
    LESS            = 14,
    BIT_OR          = 15,
    BIT_XOR         = 16,
    BIT_AND         = 17
};

enum keyword_code
{
    IF              = 0,
    ELSE            = 1,
    WHILE           = 2,
    VAR_DECL        = 3,
    RET             = 4,
    BREAK           = 5,
    CALL            = 6,
    FUNC            = 7
};

enum type_data
{
    OP      = 1,
    ID      = 2,
    NUM     = 3,
    KEYWORD = 4,
    SPEC    = 5,
    SUP     = 6
};

enum error_code
{
    NO_ERROR        = 0,
    SYNTAX_ERROR    = 1,
    TREE_NULLPTR    = 2
};

enum priority_t
{
    ZERO        = 0,
    FIRST       = 1,
    SECOND      = 2,
    THIRD       = 3,
    FOURTH      = 4,
    FIFTH       = 5,
    SIXTH       = 6,
    SEVENTH     = 7,
    EIGHTH      = 8,
    NINTH       = 9,
    TENTH       = 10,
    ELEVENTH    = 11,
    TWELFTH     = 12
};

enum support_t
{
    BODY    = 0,
    ARGS    = 1
};

enum args_number_t
{
    NO_ARGS  = 0,
    ONE_ARG  = 1,
    TWO_ARGS = 2
};

enum node_kind
{
    NODE_PROG,

    NODE_OP,       
    NODE_NUM,      
    NODE_VAR,       

    NODE_FUNC,     
    NODE_CALL,    
    NODE_BODY,     
    NODE_ARGS,
    NODE_COND,     

    NODE_IF,       
    NODE_ELSE,     
    NODE_WHILE,    
    NODE_VAR_DECL, 
    NODE_RET,      
    NODE_BREAK    
};

union data_union
{
    double number;
    int id_number;
    support_t sup_code;
    operator_code op;
    keyword_code keyword;
};

struct node_t
{
    node_kind kind;
    data_union data_t;
    node_t** children;
    size_t child_count;
    size_t child_capacity;
};

struct operator_t
{
    operator_code code;
    const char* name;
    const char* design;
    size_t strlen;
    args_number_t args_number;
    priority_t priority;
};

struct keyword_t
{
    keyword_code code;
    const char* name;
    const char* design;
    size_t strlen;
};

struct identifier_t
{
    int number;
    char* name;
    size_t length;
};

// const char* enum_to_string(type_data type);
void identifiers_destroy(identifier_t** identifiers);

bool is_close_to_zero(double number_being_checked);
// double remove_minus_before_zero(double number_being_checked);

const operator_t operators_array[] =
{
    {ADD, "ADD", "+",   1, TWO_ARGS, SECOND},
    {SUB, "SUB", "-",   1, TWO_ARGS, SECOND},
    {MUL, "MUL", "*",   1, TWO_ARGS, FIRST},
    {DIV, "DIV", "/",   1, TWO_ARGS, FIRST},
    {IS_EQUAL, "IS_EQUAL", "==", 2, TWO_ARGS, FIFTH},
    {ASSIGN, "ASSIGN", "=", 1, TWO_ARGS, TENTH},
    {LOGIC_OR, "LOGIC_OR", "||", 2, TWO_ARGS, TENTH},
    {LOGIC_AND, "LOGIC_AND", "&&", 2, TWO_ARGS, NINTH},
    {IS_NOT_EQUAL, "IS_NOT_EQUAL", "!=", 2, TWO_ARGS, FIFTH},
    {SHR, "SHR", ">>", 2, TWO_ARGS, THIRD},
    {GREATER_EQUAL, "GREATER_EQUAL", ">=", 2, TWO_ARGS, FOURTH},
    {GREATER, "GREATER", ">", 1, TWO_ARGS, FOURTH},
    {SHL, "SHL", "<<", 2, TWO_ARGS, THIRD},
    {LESS_EQUAL, "GREATER", ">=", 2, TWO_ARGS, FOURTH},
    {LESS, "LESS", "<", 1, TWO_ARGS, FOURTH},
    {BIT_OR, "BIT_OR", "|", 1, TWO_ARGS, EIGHTH},
    {BIT_XOR, "BIT_XOR", "^", 1, TWO_ARGS, SEVENTH},
    {BIT_AND, "BIT_AND", "&", 1, TWO_ARGS, SIXTH}
};

const size_t OP_ARRAY_SIZE = sizeof(operators_array)/sizeof(operators_array[0]);

const keyword_t keywords_array[] =
{
    {IF, "IF", "if", 2},
    {ELSE, "ELSE", "else", 4},
    {WHILE, "WHILE", "while", 5},
    {VAR_DECL, "VAR", "var", 3},
    {RET, "RET", "return", 6},
    {BREAK, "BREAK", "break", 5},
    {CALL, "CALL", "call", 4},
    {FUNC, "FUNC", "func", 4}
};

const size_t KEYWORD_ARRAY_SIZE = sizeof(keywords_array)/sizeof(keywords_array[0]);

#endif // FRONT_END_H
