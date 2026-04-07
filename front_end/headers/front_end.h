#ifndef FRONT_END_H
#define FRONT_END_H

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

const double NUMBER_CLOSE_TO_ZERO  = 10e-12;
const size_t LAST_CHAR_OP_NUM = 4;
const size_t FIRST_FUNC_NUM   = 5;
const size_t LAST_FUNC_NUM    = 8;

enum operator_code
{
    ADD     = 0,
    SUB     = 1,
    MUL     = 2,
    DIV     = 3,
    POW     = 4,
    LN      = 5,
    COS     = 6,
    SIN     = 7,
    EXP     = 8,
    ASSIGN  = 9
};

enum type_data
{
    OP   = 1,
    VAR  = 2,
    NUM  = 3,
    SPEC = 4
};

enum error_code
{
    NO_ERROR            = 0,
    SYNTAX_ERROR        = 1,
    TREE_NULLPTR        = 2
};

enum priority_t
{
    ZERO   = 0,
    FIRST  = 1,
    SECOND = 2,
    THIRD  = 3,
    FOURTH = 4
};

union data_union
{
    double number;
    int var_number;
    operator_code op;
};

struct node_value
{
    type_data type;
    data_union data_t;
};

struct node_t
{
    node_value* value;
    node_t* right;
    node_t* left;
};

struct operator_t
{
    operator_code code;
    const char* name;
    const char* design;
    size_t strlen;
    bool is_one_arg;
    priority_t priority;

};

struct variable_t
{
    int number;
    char* name;
    size_t length;
};

// const char* enum_to_string(type_data type);
void variables_destroy(variable_t** variables);

bool is_close_to_zero (double number_being_checked);
double remove_minus_before_zero (double number_being_checked);

const operator_t operators_array[] =
{
    {ADD, "ADD", "+",   1, false, SECOND},
    {SUB, "SUB", "-",   1, false, SECOND},
    {MUL, "MUL", "*",   1, false, FIRST},
    {DIV, "DIV", "/",   1, false, FIRST},
    {POW, "POW", "^",   1, false, ZERO},
    {LN,  "LN" , "ln",  2,  true, THIRD},
    {COS, "COS", "cos", 3,  true, THIRD},
    {SIN, "SIN", "sin", 3,  true, THIRD},
    {EXP, "EXP", "exp", 3,  true, THIRD},
    {ASSIGN, "ASSIGN", "=", 1, false, FOURTH}
};

#endif // FRONT_END_H
