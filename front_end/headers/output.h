#ifndef OUTPUT_H
#define OUTPUT_H

#include "front_end.h"
#include "input.h"

void output_to_tree(variable_t** variables_ptr, FILE* input_file, node_t** node_ptr);
void from_file_to_tree(variable_t** variables, FILE* input_file, char** buffer_ptr, char** original_ptr);
void error_message(error_code error);
const char* error_to_string(error_code error);
void program_complete(variable_t** variables_ptr, node_t** node_ptr, FILE* input_file);

#endif // OUTPUT_H
