#ifndef FRONT_END_H
#define FRONT_END_H

#include <stdbool.h>
#include <assert.h>

#include "tree.h"

#define TOKEN_INFO(code, name, design) {code, name, design, sizeof(design) - 1}

const size_t FIRST_CHILDREN_NUMBER  = 3;

enum error_code
{
    NO_ERROR = 0,
    SYNTAX_ERROR,
    TREE_NULLPTR,
    MULTIPLE_VAR_DECLARATION,
    PROG_NODE_ERROR,
    UNDECLARED_VARIABLE,
    UNDECLARED_FUNCTION,
    MULTIPLE_FUNC_DECLARATION
};

struct token_info_t
{
    int code;
    const char* name;
    const char* design;
    size_t strlen;
};

void identifiers_destroy(identifier_t** identifiers);

const token_info_t operators_array[] =
{
    TOKEN_INFO(ADD,           "ADD",               "+"),
    TOKEN_INFO(SUB,           "SUB",               "-"),
    TOKEN_INFO(MUL,           "MUL",               "*"),
    TOKEN_INFO(DIV,           "DIV",               "/"),
    TOKEN_INFO(IS_EQUAL,      "IS_EQUAL",         "=="),
    TOKEN_INFO(ASSIGN,        "ASSIGN",            "="),
    TOKEN_INFO(LOGIC_OR,      "LOGIC_OR",         "||"),
    TOKEN_INFO(LOGIC_AND,     "LOGIC_AND",        "&&"),
    TOKEN_INFO(IS_NOT_EQUAL,  "IS_NOT_EQUAL",     "!="),
    TOKEN_INFO(SHR,           "SHR",              ">>"),
    TOKEN_INFO(GREATER_EQUAL, "GREATER_EQUAL",    ">="),
    TOKEN_INFO(GREATER,       "GREATER",           ">"),
    TOKEN_INFO(SHL,           "SHL",              "<<"),
    TOKEN_INFO(LESS_EQUAL,    "LESS_EQUAL",       "<="),
    TOKEN_INFO(LESS,          "LESS",              "<"),
    TOKEN_INFO(BIT_OR,        "BIT_OR",            "|"),
    TOKEN_INFO(BIT_XOR,       "BIT_XOR",           "^"),
    TOKEN_INFO(BIT_AND,       "BIT_AND",           "&")
};

const size_t OP_ARRAY_SIZE = sizeof(operators_array)/sizeof(operators_array[0]);

const token_info_t keywords_array[] =
{
    TOKEN_INFO(IF,        "IF",         "if"),
    TOKEN_INFO(ELSE,      "ELSE",     "else"),
    TOKEN_INFO(WHILE,     "WHILE",   "while"),
    TOKEN_INFO(VAR_DECL,  "VAR",       "var"),
    TOKEN_INFO(RET,       "RET",    "return"),
    TOKEN_INFO(BREAK,     "BREAK",   "break"),
    TOKEN_INFO(CALL,      "CALL",     "call"),
    TOKEN_INFO(FUNC,      "FUNC",     "func")
};

const size_t KEYWORD_ARRAY_SIZE = sizeof(keywords_array)/sizeof(keywords_array[0]);

const token_info_t specs_array[] =
{
    TOKEN_INFO(LEFT_BRACE,    "LEFT_BRACE",   "{"),
    TOKEN_INFO(RIGHT_BRACE,   "RIGHT_BRACE",  "}"),
    TOKEN_INFO(LEFT_PAREN,    "LEFT_PAREN",   "("),
    TOKEN_INFO(RIGHT_PAREN,   "RIGHT_PAREN",  ")"),
    TOKEN_INFO(SEMMICOLON,    "SEMMICOLON",   ";"),
    TOKEN_INFO(COMMA,         "COMMA",        ",")
};

const size_t SPEC_ARRAY_SIZE = sizeof(specs_array) / sizeof(specs_array[0]);

node_t* front_end_run(FILE* input_file, identifier_t** identifiers);

#endif // FRONT_END_H
