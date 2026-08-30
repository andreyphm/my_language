#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "instructions_to_nasm.h"
#include "font.h"

static const size_t COPY_BUFFER_SIZE = 4096;

static void copy_stdlib(FILE* destination, const char* source_path);
static void print_labels(FILE* output_file, const label_list_t* labels, size_t instruction_index);
static void print_instruction(FILE* output_file, const instruction_t* instruction);
static void print_operand(FILE* output_file, const operand_t* operand);
static const char* register_name(size_t number, size_t size);

void instructions_to_nasm(const instruction_list_t* instructions, const label_list_t* labels,
                          FILE* output_file, const char* stdlib_source_path)
{
    assert(instructions);
    assert(labels);
    assert(output_file);

    fprintf(output_file,
            ";*******************************************************;\n"
            ";==================== PROGRAM START ====================;\n"
            ";*******************************************************;\n\n"
            "BITS 64\n"
            "global main\n"
            "section .text\n\n");

    if (stdlib_source_path)
    {
        copy_stdlib(output_file, stdlib_source_path);
        fprintf(output_file, "\n\n");
    }

    bool rodata_started = false;
    for (size_t i = 0; i < instructions->count; i++)
    {
        if (!rodata_started && !strcmp(instructions->instructions[i].mnemonic, "dq"))
        {
            fprintf(output_file, "\nsection .rodata\n\n");
            rodata_started = true;
        }

        print_labels(output_file, labels, i);
        print_instruction(output_file, &instructions->instructions[i]);
    }

    print_labels(output_file, labels, instructions->count);
    // printf(MAKE_BOLD_GREEN("Instructions to NASM successful\n"));
}

static void copy_stdlib(FILE* destination, const char* source_path)
{
    assert(destination);
    assert(source_path);

    FILE* source = fopen(source_path, "rb");
    assert(source);

    char first_line[64] = {};
    char* line_read = fgets(first_line, sizeof(first_line), source);
    assert(line_read);

    if (strcmp(first_line, "BITS 64\n") &&
        strcmp(first_line, "BITS 64\r\n") &&
        strcmp(first_line, "BITS 64"))
    {
        size_t line_size = strlen(first_line);
        size_t bytes_written = fwrite(first_line, sizeof(char), line_size, destination);
        assert(bytes_written == line_size);
    }

    char buffer[COPY_BUFFER_SIZE] = {};
    size_t bytes_read = 0;
    while ((bytes_read = fread(buffer, sizeof(char), sizeof(buffer), source)) > 0)
    {
        size_t bytes_written = fwrite(buffer, sizeof(char), bytes_read, destination);
        assert(bytes_written == bytes_read);
    }

    assert(!ferror(source));

    fclose(source);
}

static void print_labels(FILE* output_file, const label_list_t* labels, size_t instruction_index)
{
    assert(output_file);
    assert(labels);

    for (size_t i = 0; i < labels->count; i++)
    {
        if (labels->labels[i].instruction_index == instruction_index)
            fprintf(output_file, "%s:\n", labels->labels[i].name);
    }
}

static void print_instruction(FILE* output_file, const instruction_t* instruction)
{
    assert(output_file);
    assert(instruction);

    if (instruction->mnemonic[0] == '\0')
    {
        fprintf(output_file, "\n; %s\n", instruction->comment);
        return;
    }

    fprintf(output_file, "\t%s", instruction->mnemonic);
    for (size_t i = 0; i < instruction->operand_count; i++)
    {
        fprintf(output_file, "%s", i == 0 ? " " : ", ");
        print_operand(output_file, &instruction->operands[i]);
    }
    if (instruction->comment[0] != '\0')
        fprintf(output_file, "\t; %s\n", instruction->comment);
    else
        fputc('\n', output_file);
}

static void print_operand(FILE* output_file, const operand_t* operand)
{
    assert(output_file);
    assert(operand);

    switch (operand->kind)
    {
        case OPERAND_REG:
            fprintf(output_file, "%s", register_name(operand->reg_num, operand->reg_size));
            break;

        case OPERAND_XMM:
            fprintf(output_file, "xmm%zu", operand->reg_num);
            break;

        case OPERAND_MEM:
            fprintf(output_file, "[%s", register_name(operand->reg_num, 8));
            if (operand->displacement > 0)
                fprintf(output_file, " + %" PRId64, operand->displacement);
            else if (operand->displacement < 0)
                fprintf(output_file, " - %" PRId64, -operand->displacement);
            fputc(']', output_file);
            break;

        case OPERAND_MEM_REL:
            fprintf(output_file, "[rel %s]", operand->label_name);
            break;

        case OPERAND_IMM:
            fprintf(output_file, "%" PRId64, operand->imm_value);
            break;

        case OPERAND_DOUBLE:
            fprintf(output_file, "%#.17g", operand->double_value);
            break;

        case OPERAND_LABEL:
            fprintf(output_file, "%s", operand->label_name);
            break;

        case NO_OPERAND:
        default:
            assert(0);
    }
}

static const char* register_name(size_t number, size_t size)
{
    static const char* registers_64[] = {"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi"};
    static const char* registers_32[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
    static const char* registers_8[]  = {"al",  "cl",  "dl",  "bl",  "spl", "bpl", "sil", "dil"};

    assert(number < sizeof(registers_64) / sizeof(registers_64[0]));
    if (size == 8) return registers_64[number];
    if (size == 4) return registers_32[number];
    if (size == 1) return registers_8[number];

    assert(0 && "Unknown register");
}
