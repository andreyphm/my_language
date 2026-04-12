#ifndef OUTPUT_H
#define OUTPUT_H

#include "front_end.h"
#include "input.h"

void output_to_tree(identifier_t** identifiers_ptr, FILE* input_file, node_t** node_ptr);
void from_file_to_tree(identifier_t** identifiers, FILE* input_file, char** buffer_ptr, char** original_ptr);
void error_message(error_code error);
const char* error_to_string(error_code error);
void program_complete(identifier_t** identifiers_ptr, node_t** node_ptr, FILE* input_file);

#endif // OUTPUT_H
