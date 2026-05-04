#ifndef TREE_H
#define TREE_H

#include <stdio.h>

#define TREE_DUMP_TXT "tree/tree_dump/tree_dump.txt"
#define TREE_DUMP_PNG "tree/tree_dump/tree_dump.png"

enum operator_code
{
    ADD,
    SUB,
    MUL,
    DIV,
    IS_EQUAL,
    ASSIGN,
    LOGIC_OR,
    LOGIC_AND,
    IS_NOT_EQUAL,
    SHR,
    GREATER_EQUAL,
    GREATER,
    SHL,
    LESS_EQUAL,
    LESS,
    BIT_OR,
    BIT_XOR,
    BIT_AND
};

enum keyword_code
{
    IF,
    ELSE,
    WHILE,
    VAR_DECL,
    RET,
    BREAK,
    CALL,
    FUNC
};

enum spec_code
{
    PROGRAM_START,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_PAREN,
    RIGHT_PAREN,
    SEMMICOLON,
    COMMA,
    PROGRAM_END
};

enum type_data
{
    OP,
    ID,
    NUM,
    KEYWORD,
    SPEC
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

struct var_t
{
    int id_number;
    int unique_id;
    size_t stack_offset;
};

struct func_t
{
    int id_number;
    size_t frame_size;
};

union data_union
{
    double number;
    operator_code op;
    keyword_code keyword;
    func_t function;
    var_t variable;
};

struct node_t
{
    node_kind kind;
    data_union data_t;
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
node_t* create_num_node(double value);
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
