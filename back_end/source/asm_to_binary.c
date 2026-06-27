#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "asm_to_binary.h"
#include "asm_dump.h"

static char* read_file_to_buffer(FILE* const tree_txt_file);
static void skip_line(char** string);
static void skip_spaces(char** string);

void asm_to_binary(FILE* const asm_file, FILE* const binary_file)
{
    assert(asm_file);
    assert(binary_file);

    char* asm_buffer = read_file_to_buffer(asm_file);

    instruction_list_t instruction_list = {};
    label_list_t label_list = {};
    asm_code_to_instructions(asm_buffer, &instruction_list, &label_list);

    asm_dump(&instruction_list, &label_list, ASM_DUMP_TXT, ASM_DUMP_PNG);

    // Elf64_Ehdr elf_header = {};
    // fill_elf_header(&elf_header, ENTRY_POINT);

    // Elf64_Phdr program_header[SEGMENT_COUNT] = {};
    // fill_program_header(program_header, BASE_VADDR, CODE_OFFSET, , , );

    free(asm_buffer);
    instruction_list_destroy(&instruction_list);
    label_list_destroy(&label_list);
}

static char* read_file_to_buffer(FILE* const tree_txt_file)
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

    if (**asm_buffer == ';' || **asm_buffer == '%' || !strncmp(*asm_buffer, "global", sizeof("global") - 1))
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
    while (**asm_buffer != ':' && **asm_buffer != '\n' && **asm_buffer != '\0')
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
    while (isalpha(**asm_buffer) || **asm_buffer == '_')
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

    if (!isdigit(**asm_buffer))
        return false;

    char* end = nullptr;
    double value = strtod(*asm_buffer, &end);

    bool has_dot = false;
    for (char* symbol_ptr = *asm_buffer; symbol_ptr < end; symbol_ptr++)
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

    operand->kind = OPERAND_MEM;
    try_reg(asm_buffer, operand);
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

    if (**asm_buffer == '.' || isalpha(**asm_buffer))
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
    list->labels = nullptr;
    list->count    = 0;
    list->capacity = 0;
}

void fill_elf_header(Elf64_Ehdr* header, uint64_t entry_point)
{
    assert(header);

    memset(header, 0, sizeof(Elf64_Ehdr));

    header->e_ident[EI_MAG0]        = ELFMAG0;          // 0x7F - magic number
    header->e_ident[EI_MAG1]        = ELFMAG1;          // 'E'
    header->e_ident[EI_MAG2]        = ELFMAG2;          // 'L'
    header->e_ident[EI_MAG3]        = ELFMAG3;          // 'F'
    header->e_ident[EI_CLASS]       = ELFCLASS64;
    header->e_ident[EI_DATA]        = ELFDATA2LSB;      // little-endian
    header->e_ident[EI_VERSION]     = EV_CURRENT;       // ELF format version
    header->e_ident[EI_OSABI]       = ELFOSABI_LINUX;   // ABI
    header->e_ident[EI_ABIVERSION]  = 0;
    // remaining bytes of e_ident are zeroed out by memset

    header->e_type      = ET_EXEC;              // execulist file
    header->e_machine   = EM_X86_64;            // architecture (x86-64)
    header->e_version   = EV_CURRENT;           // object file version
    header->e_entry     = entry_point;
    header->e_phoff     = sizeof(Elf64_Ehdr);   // Program Header offset (after ELF header)
    header->e_shoff     = 0;                    // !Section Header offset
    header->e_flags     = 0;
    header->e_ehsize    = sizeof(Elf64_Ehdr);   // size of ELF header
    header->e_phentsize = sizeof(Elf64_Phdr);   // size of Program Header element
    header->e_phnum     = SEGMENT_COUNT;        // number of segments
    header->e_shentsize = sizeof(Elf64_Shdr);   // size of Section Header element
    header->e_shnum     = 0;                    // !number of sections
    header->e_shstrndx  = 0;                    // !section with string list index
}

void fill_program_header(Elf64_Phdr* header, uint64_t base_vaddr, uint64_t code_offset, uint64_t code_size,
                                                                  uint64_t data_offset, uint64_t data_size)
{
    assert(header);

    memset(header, 0, 2 * sizeof(Elf64_Phdr));

    // first segment (.text):
    header[0].p_type   = PT_LOAD;                  // load segment to memory
    header[0].p_flags  = PF_R | PF_X;              // read + execute
    header[0].p_offset = 0;                        // segment offset
    header[0].p_vaddr  = base_vaddr;               // virtual address
    header[0].p_paddr  = base_vaddr;               // physical address (basically ignored)
    header[0].p_filesz = code_offset + code_size;  // size of segment in file
    header[0].p_memsz  = code_offset + code_size;  // size of segment in memory (no .bss)
    header[0].p_align  = 0x1000;                   // page size

    // second segment (.data):
    header[1].p_type   = PT_LOAD;                  // load segment to memory
    header[1].p_flags  = PF_R | PF_W;              // read + write
    header[1].p_offset = data_offset;              // segment offset
    header[1].p_vaddr  = base_vaddr + data_offset; // virtual address
    header[1].p_paddr  = base_vaddr + data_offset; // physical address (basically ignored)
    header[1].p_filesz = data_size;                // size of segment in file
    header[1].p_memsz  = data_size;                // size of segment in memory (no .bss)
    header[1].p_align  = 0x1000;                   // page size
}
