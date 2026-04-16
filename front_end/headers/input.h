#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>

const int CORRECT_NUMBER_OF_FILES = 3;

void check_files(FILE** const input_file, FILE** const output_file, int argc, const char* const argv[]);
void bad_argc_message(const char* const* argv);

void clear_input_buffer();
char* read_file_to_buffer(FILE* const tree_txt_file);

#endif // INPUT_H
