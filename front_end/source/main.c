#include "front_end.h"
#include "input.h"
#include "tokenization.h"
#include "parser.h"
#include "output.h"

int main(int argc, const char* argv[])
{
    FILE* input_file = nullptr;
    FILE* output_file = nullptr;
    check_files(&input_file, &output_file, argc, argv);

    error_code error = NO_ERROR;
    node_t* node = nullptr;
    variable_t* variables = nullptr;
    list_t list = {nullptr, nullptr, nullptr};

    file_to_tokens(&variables, input_file, &list);
    if (error) 
    {
        error_message(error);
        return 0;
    }

    tokens_to_tree(&list, &node);
    if (error) 
    {
        error_message(error);
        return 0;
    }

    program_complete(&variables, &node, input_file);
}
