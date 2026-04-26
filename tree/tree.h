#ifndef TREE_H
#define TREE_H

#include <stdio.h>

#define TREE_DUMP_TXT "tree/tree_dump/tree_dump.txt"
#define TREE_DUMP_PNG "tree/tree_dump/tree_dump.png"

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
    SPEC    = 5
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
    int number;
    int id_number;
    operator_code op;
    keyword_code keyword;
};

struct node_t
{
    node_kind kind;
    data_union data_t;
    int unique_id;
    node_t** children;
    size_t child_count;
    size_t child_capacity;
};

struct identifier_t
{
    int number;
    char* name;
    size_t length;
};

node_t* create_node(node_kind kind, data_union data);
node_t* copy_node(node_t* node);
void node_reserve(node_t* node, size_t new_capacity);
void node_add_child(node_t* parent, node_t* child);

node_t* create_prog_node();
node_t* create_num_node(int value);
node_t* create_var_node(int var_id);
node_t* create_op_node(operator_code op, node_t* left, node_t* right);
node_t* create_body_node();
node_t* create_args_node();
node_t* create_func_node(int func_id_num, node_t* args, node_t* body);
node_t* create_call_node(int func_id_num, node_t* args);
node_t* create_if_node(node_t* condition, node_t* then_body, node_t* else_body);
node_t* create_else_node();
node_t* create_while_node(node_t* condition, node_t* body);
node_t* create_var_decl_node(node_t* var, node_t* init);
node_t* create_ret_node(node_t* expr);
node_t* create_break_node();

node_t* destroy_and_null(node_t* node);
void destroy_node(node_t* node);

void tree_dump(node_t* const node, const char* const png_file_name, const identifier_t* const identifiers);

#endif // TREE_H
