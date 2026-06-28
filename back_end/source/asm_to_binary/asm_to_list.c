#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "asm_to_binary.h"

static void skip_line(char** string);
static void skip_spaces(char** string);

void asm_code_to_instructions(char* asm_buffer, instruction_list_t* const instruction_list, label_list_t* label_list)
{
    assert(asm_buffer);
    assert(instruction_list);
    assert(label_list);

    instruction_list_init(instruction_list);
    label_list_init(label_list);
    section_kind current_section = NO_SECTION;

    while (*asm_buffer != '\0')
    {
        skip_spaces(&asm_buffer);
        if (*asm_buffer == '\n')
        {
            asm_buffer++;
            continue;
        }
        if (*asm_buffer == '\0') break;

        parse_instruction(&asm_buffer, instruction_list, label_list, &current_section);
    }
}

void parse_instruction(char** asm_buffer, instruction_list_t* const instruction_list,
                                          label_list_t* label_list, section_kind* section)
{
    assert(asm_buffer);
    assert(*asm_buffer);
    assert(instruction_list);
    assert(label_list);
    assert(section);

    if (**asm_buffer == ';')
    {
        skip_line(asm_buffer);
        return;
    }

    if (try_section(asm_buffer, section) ||
        try_label(asm_buffer, instruction_list, label_list))
        return;

    instruction_t instruction = {};
    read_mnemonic(asm_buffer, &instruction);
    skip_spaces(asm_buffer);

    while (**asm_buffer != '\n' && **asm_buffer != '\0' && **asm_buffer != ';')
    {
        if (instruction.operand_count >= 2)
        {
            printf("Too many operands! Mnemonic: '%s', at: '%.30s'\n", instruction.mnemonic, *asm_buffer);
            assert(0);
        }

        parse_operand(asm_buffer, &instruction.operands[instruction.operand_count++]);
        skip_spaces(asm_buffer);

        if (**asm_buffer == ',')
        {
            (*asm_buffer)++;
            skip_spaces(asm_buffer);
        }
    }

    instruction_list_push_back(instruction_list, instruction);
}

void parse_operand(char** asm_buffer, operand_t* operand)
{
    assert(asm_buffer);
    assert(*asm_buffer);
    assert(operand);

    if (try_double(asm_buffer, operand) ||
        try_imm(asm_buffer, operand)    ||
        try_xmm(asm_buffer, operand)    ||
        try_reg(asm_buffer, operand)    ||
        try_mem(asm_buffer, operand)    ||
        try_label_jump(asm_buffer, operand))
        return;

    printf("Unknown operand at: '%.30s'\n", *asm_buffer);
    assert(0);
}

bool try_section(char** asm_buffer, section_kind* section)
{
    assert(asm_buffer);
    assert(*asm_buffer);
    assert(section);

    if (!strncmp(*asm_buffer, "section", sizeof("section") - 1))
    {
        *asm_buffer += sizeof("section") - 1;
        skip_spaces(asm_buffer);

        if (!strncmp(*asm_buffer, ".text", sizeof(".text") - 1))
            *section = SECTION_TEXT;
        else if (!strncmp(*asm_buffer, ".rodata", sizeof(".rodata") - 1))
            *section = SECTION_RODATA;

        skip_line(asm_buffer);
        return true;
    }

    return false;
}

bool try_label(char** asm_buffer, instruction_list_t* const instruction_list, label_list_t* label_list)
{
    assert(asm_buffer);
    assert(*asm_buffer);
    assert(instruction_list);
    assert(label_list);

    char* line_start = *asm_buffer;
    while (**asm_buffer != ':' && **asm_buffer != '\n' && **asm_buffer != '\0' && **asm_buffer != ';')
        (*asm_buffer)++;

    if (**asm_buffer == ':')
    {
        label_t label = {};
        size_t name_len = (size_t) (*asm_buffer - line_start);
        strncpy(label.name, line_start, name_len);
        label.instruction_index = instruction_list->count;

        label_list_push_back(label_list, label);

        (*asm_buffer)++;
        return true;
    }

    *asm_buffer = line_start;
    return false;
}

void read_mnemonic(char** asm_buffer, instruction_t* instruction)
{
    assert(asm_buffer);
    assert(*asm_buffer);
    assert(instruction);

    const char* line_start = *asm_buffer;
    while (isalpha(**asm_buffer) || **asm_buffer == '_' || isdigit(**asm_buffer))
        (*asm_buffer)++;

    size_t mnemonic_len = (size_t) (*asm_buffer - line_start);
    assert(mnemonic_len < sizeof(instruction->mnemonic));
    strncpy(instruction->mnemonic, line_start, mnemonic_len);
}

bool try_double(char** asm_buffer, operand_t* operand)
{
    assert(asm_buffer);
    assert(*asm_buffer);
    assert(operand);

    char* start = *asm_buffer;

    if (**asm_buffer == '-')
        (*asm_buffer)++;

    if (!isdigit(**asm_buffer))
    {
        *asm_buffer = start;
        return false;
    }

    char* end = nullptr;
    double value = strtod(*asm_buffer, &end);

    bool has_dot = false;
    for (char* symbol_ptr = start; symbol_ptr < end; symbol_ptr++)
    {
        if (*symbol_ptr == '.')
        {
            has_dot = true;
            break;
        }
    }

    if (!has_dot)
        return false;

    operand->kind = OPERAND_DOUBLE;
    operand->double_value = value;
    *asm_buffer = end;
    return true;
}

bool try_imm(char** asm_buffer, operand_t* operand)
{
    assert(asm_buffer);
    assert(*asm_buffer);
    assert(operand);

    if (**asm_buffer == '\'')
    {
        operand->kind = OPERAND_IMM;
        (*asm_buffer)++;
        operand->imm_value = (int64_t) (**asm_buffer);
        (*asm_buffer)++;
        assert(**asm_buffer == '\'');
        (*asm_buffer)++;
        return true;
    }

    if (isdigit(**asm_buffer))
    {
        operand->kind = OPERAND_IMM;
        operand->imm_value = 0;

        while (isdigit(**asm_buffer))
        {
            operand->imm_value = operand->imm_value * 10 + (**asm_buffer - '0');
            (*asm_buffer)++;
        }
        return true;
    }

    return false;
}

bool try_xmm(char** asm_buffer, operand_t* operand)
{
    assert(asm_buffer);
    assert(*asm_buffer);
    assert(operand);

    if (!strncmp(*asm_buffer, "xmm", sizeof("xmm") - 1) &&
        !isalnum((*asm_buffer)[sizeof("xmm")]) &&
        (*asm_buffer)[sizeof("xmm")] != '_')
    {
        *asm_buffer += sizeof("xmm") - 1;
        operand->kind = OPERAND_XMM;
        operand->reg_num = (size_t) (**asm_buffer - '0');
        (*asm_buffer)++;
        return true;
    }

    return false;
}

bool try_reg(char** asm_buffer, operand_t* operand)
{
    assert(asm_buffer);
    assert(*asm_buffer);
    assert(operand);

    for (size_t i = 0; i < REG_ARRAY_SIZE; i++)
    {
        size_t name_len = strlen(registers_array[i].name);
        if (!strncmp(*asm_buffer, registers_array[i].name, name_len) &&
            !isalnum((*asm_buffer)[name_len]) &&
            (*asm_buffer)[name_len] != '_')
        {
            operand->kind = OPERAND_REG;
            operand->reg_num = registers_array[i].number;
            operand->reg_size = registers_array[i].size;
            *asm_buffer += name_len;
            return true;
        }
    }

    return false;
}

bool try_mem(char** asm_buffer, operand_t* operand)
{
    assert(asm_buffer);
    assert(*asm_buffer);
    assert(operand);

    if (**asm_buffer != '[')
        return false;

    (*asm_buffer)++;
    skip_spaces(asm_buffer);

    if (try_mem_rel(asm_buffer, operand))
        return true;

    try_reg(asm_buffer, operand);
    operand->kind = OPERAND_MEM;
    skip_spaces(asm_buffer);

    if (**asm_buffer == '+' || **asm_buffer == '-')
    {
        int sign = (**asm_buffer == '+') ? 1 : -1;
        (*asm_buffer)++;
        skip_spaces(asm_buffer);

        int64_t displacement = 0;
        while (isdigit(**asm_buffer))
        {
            displacement = displacement * 10 + (**asm_buffer - '0');
            (*asm_buffer)++;
        }
        operand->displacement = sign * displacement;
    }

    skip_spaces(asm_buffer);
    assert(**asm_buffer == ']');
    (*asm_buffer)++;
    return true;
}

bool try_mem_rel(char** asm_buffer, operand_t* operand)
{
    assert(asm_buffer);
    assert(*asm_buffer);
    assert(operand);

    if (!strncmp(*asm_buffer, "rel", sizeof("rel") - 1))
    {
        *asm_buffer += sizeof("rel") - 1;
        skip_spaces(asm_buffer);

        operand->kind = OPERAND_MEM_REL;
        const char* start = *asm_buffer;
        while (isalnum(**asm_buffer) || **asm_buffer == '_')
            (*asm_buffer)++;

        size_t len = (size_t) (*asm_buffer - start);
        assert(len < sizeof(operand->label_name));
        strncpy(operand->label_name, start, len);

        skip_spaces(asm_buffer);
        assert(**asm_buffer == ']');
        (*asm_buffer)++;
        return true;
    }

    return false;
}

bool try_label_jump(char** asm_buffer, operand_t* operand)
{
    assert(asm_buffer);
    assert(*asm_buffer);
    assert(operand);

    if (**asm_buffer == '.' || **asm_buffer == '_' || isalpha(**asm_buffer))
    {
        operand->kind = OPERAND_LABEL;
        const char* label_start = *asm_buffer;
        while (isalpha(**asm_buffer) || isdigit(**asm_buffer) || **asm_buffer == '_' || **asm_buffer == '.')
            (*asm_buffer)++;
        
        size_t name_len = (size_t) (*asm_buffer - label_start);
        assert(name_len < sizeof(operand->label_name));
        strncpy(operand->label_name, label_start, name_len);
        return true;
    }

    return false;
}

static void skip_line(char** string)
{
    while (**string != '\n' && **string != '\0')
        (*string)++;

    if (**string == '\n')
        (*string)++;
}

static void skip_spaces(char** string)
{
    while (**string == ' ' || **string == '\t')
        (*string)++;
}

void instruction_list_init(instruction_list_t* const list)
{
    assert(list);

    list->instructions  = (instruction_t*) calloc(INITIAL_INSTRUCTIONS_CAPACITY, sizeof(instruction_t));
    list->count         = 0;
    list->capacity      = INITIAL_INSTRUCTIONS_CAPACITY;
}

void instruction_list_push_back(instruction_list_t* const list, instruction_t instruction)
{
    assert(list);

    if (list->count == list->capacity)
    {
        list->capacity *= 2;
        list->instructions = (instruction_t*) realloc(list->instructions,
                                                      list->capacity * sizeof(instruction_t));
        assert(list->instructions);
    }

    list->instructions[list->count++] = instruction;
}

void instruction_list_destroy(instruction_list_t* const list)
{
    assert(list);

    free(list->instructions);
    list->instructions = nullptr;
    list->count    = 0;
    list->capacity = 0;
}

void label_list_init(label_list_t* list)
{
    assert(list);

    list->labels   = (label_t*) calloc(INITIAL_LABELS_CAPACITY, sizeof(label_t));
    list->count    = 0;
    list->capacity = INITIAL_LABELS_CAPACITY;
}

void label_list_push_back(label_list_t* list, label_t label)
{
    assert(list);

    if (list->count == list->capacity)
    {
        list->capacity *= 2;
        list->labels = (label_t*) realloc(list->labels, list->capacity * sizeof(label_t));
        assert(list->labels);
    }

    list->labels[list->count++] = label;
}

void label_list_destroy(label_list_t* list)
{
    assert(list);

    free(list->labels);
    list->labels   = nullptr;
    list->count    = 0;
    list->capacity = 0;
}
