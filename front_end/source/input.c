#include <assert.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "front_end.h"
#include "font.h"
#include "input.h"

#define DEFAULT_INPUT_FILE  "input.txt"
#define DEFAULT_OUTPUT_FILE "output.txt"

void clear_input_buffer()
{
    int entered_character = 0;

    do {
        entered_character = getchar();
    } while (entered_character != '\n' && entered_character != EOF);
}

char* read_file_to_buffer(FILE* const tree_txt_file)
{
    assert(tree_txt_file);

    struct stat file_struct = {};
    fstat(fileno(tree_txt_file), &file_struct);
    size_t file_size = (size_t)file_struct.st_size;

    char* buffer = (char*) calloc(file_size + 1, sizeof(*buffer));
    file_size = fread(buffer, sizeof(*buffer), file_size, tree_txt_file);
    buffer[file_size] = '\0';

    return buffer;
}

void bad_argc_message(const char* const* argv)
{
    printf(MAKE_BOLD("You haven't entered the input and output files or you entered them incorrectly."
                        "\nDefault files will be used: " 
                        DEFAULT_INPUT_FILE " for input and " 
                        DEFAULT_OUTPUT_FILE " for output.\nIf you want to select your files, please, "
                        "use: %s input_file output_file.\n\n"), argv[0]);
}

void check_files(FILE** const input_file, FILE** const output_file, int argc, const char* const argv[])
{
    if (argc == CORRECT_NUMBER_OF_FILES)
    {
        *input_file = fopen(argv[1], "r");
        *output_file = fopen(argv[2], "w");

        if (!*input_file)
        {
            printf(MAKE_BOLD_RED("Can't open input file. Default input file will be used: " DEFAULT_INPUT_FILE ".\n"));
            *input_file = fopen(DEFAULT_INPUT_FILE, "r");
        }

        if (!*output_file)
        {
            printf(MAKE_BOLD_RED("Can't open output file. Default output file will be used: " DEFAULT_OUTPUT_FILE ".\n"));
            *output_file = fopen(DEFAULT_OUTPUT_FILE, "w");
        }
    }
    else
    {
        bad_argc_message(argv);
        *input_file = fopen(DEFAULT_INPUT_FILE, "r");
        *output_file = fopen(DEFAULT_OUTPUT_FILE, "w");
    }
}
