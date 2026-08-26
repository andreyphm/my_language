#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "instructions_to_binary.h"

static uint8_t* read_binary_file(const char* path, size_t* file_size);
static void add_stdlib_label(label_list_t* labels, const char* name, uint64_t address);
static size_t find_rodata_instruction_index(const instruction_list_t* instructions);
static size_t instructions_size_between(const instruction_list_t* instructions,
                                        size_t begin, size_t end);
static uint64_t align_up(uint64_t value, uint64_t alignment);
static void write_zero_padding(FILE* file, size_t padding_size);

static const size_t STDLIB_OUT_OFFSET = 0;
static const size_t STDLIB_IN_OFFSET  = 5;

void instructions_to_binary(instruction_list_t* instruction_list, label_list_t* label_list,
                            FILE* const binary_file, const char* stdlib_binary_path)
{
    assert(instruction_list);
    assert(label_list);
    assert(binary_file);

    calculate_sizes(instruction_list);
    size_t rodata_index = find_rodata_instruction_index(instruction_list);
    size_t text_size = instructions_size_between(instruction_list, 0, rodata_index);
    size_t rodata_size = instructions_size_between(instruction_list, rodata_index,
                                                   instruction_list->count);

    size_t stdlib_size = 0;
    uint8_t* stdlib_buffer = nullptr;
    if (stdlib_binary_path)
        stdlib_buffer = read_binary_file(stdlib_binary_path, &stdlib_size);

    size_t text_payload_size = text_size + stdlib_size;
    uint64_t text_file_size = CODE_OFFSET + text_payload_size;
    uint64_t rodata_offset = align_up(text_file_size, ELF_PAGE_SIZE);
    uint64_t text_start = BASE_VADDR + CODE_OFFSET;
    uint64_t rodata_start = BASE_VADDR + rodata_offset;

    compute_labels_addresses(instruction_list, label_list, rodata_index,
                             text_start, rodata_start);

    if (stdlib_buffer)
    {
        uint64_t stdlib_address = text_start + text_size;
        add_stdlib_label(label_list, "__out",  stdlib_address + STDLIB_OUT_OFFSET);
        add_stdlib_label(label_list, "__in",   stdlib_address + STDLIB_IN_OFFSET);
    }

    instruction_list_t text_instructions = {};
    text_instructions.instructions = instruction_list->instructions;
    text_instructions.count = rodata_index;
    text_instructions.capacity = rodata_index;

    instruction_list_t rodata_instructions = {};
    rodata_instructions.instructions = instruction_list->instructions + rodata_index;
    rodata_instructions.count = instruction_list->count - rodata_index;
    rodata_instructions.capacity = rodata_instructions.count;

    uint8_t* text_buffer = (uint8_t*) calloc(text_payload_size, sizeof(uint8_t));
    assert(text_buffer);
    encode_all(&text_instructions, label_list, text_start, text_buffer);
    if (stdlib_size > 0)
        memcpy(text_buffer + text_size, stdlib_buffer, stdlib_size);

    uint8_t* rodata_buffer = nullptr;
    if (rodata_size > 0)
    {
        rodata_buffer = (uint8_t*) calloc(rodata_size, sizeof(uint8_t));
        assert(rodata_buffer);
        encode_all(&rodata_instructions, label_list, rodata_start, rodata_buffer);
    }

    Elf64_Ehdr elf_header = {};
    uint64_t entry_point = find_label_address(label_list, "main");
    fill_elf_header(&elf_header, entry_point);

    Elf64_Phdr program_header[SEGMENT_COUNT] = {};
    fill_program_headers(program_header, BASE_VADDR, text_file_size,
                         rodata_offset, rodata_size);

    size_t written = fwrite(&elf_header, sizeof(Elf64_Ehdr), 1, binary_file);
    assert(written == 1);
    written = fwrite(program_header, sizeof(Elf64_Phdr), SEGMENT_COUNT, binary_file);
    assert(written == SEGMENT_COUNT);
    written = fwrite(text_buffer, 1, text_payload_size, binary_file);
    assert(written == text_payload_size);
    write_zero_padding(binary_file, rodata_offset - text_file_size);
    if (rodata_size > 0)
    {
        written = fwrite(rodata_buffer, 1, rodata_size, binary_file);
        assert(written == rodata_size);
    }

    free(text_buffer);
    free(rodata_buffer);
    free(stdlib_buffer);
}

static uint8_t* read_binary_file(const char* path, size_t* file_size)
{
    assert(path);
    assert(file_size);

    FILE* file = fopen(path, "rb");
    assert(file);

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    assert(size >= 0);
    rewind(file);

    *file_size = (size_t) size;
    uint8_t* buffer = (uint8_t*) calloc(*file_size, sizeof(uint8_t));
    assert(buffer);
    size_t bytes_read = fread(buffer, 1, *file_size, file);
    assert(bytes_read == *file_size);

    fclose(file);
    return buffer;
}

static void add_stdlib_label(label_list_t* labels, const char* name, uint64_t address)
{
    assert(labels);
    assert(name);

    label_t label = {};
    strncpy(label.name, name, sizeof(label.name) - 1);
    label.address = address;
    label.instruction_index = labels->count;
    label_list_push_back(labels, label);
}

static size_t find_rodata_instruction_index(const instruction_list_t* instructions)
{
    assert(instructions);

    for (size_t i = 0; i < instructions->count; i++)
    {
        if (!strcmp(instructions->instructions[i].mnemonic, "dq"))
            return i;
    }

    return instructions->count;
}

static size_t instructions_size_between(const instruction_list_t* instructions,
                                        size_t begin, size_t end)
{
    assert(instructions);
    assert(begin <= end);
    assert(end <= instructions->count);

    size_t size = 0;
    for (size_t i = begin; i < end; i++)
        size += instructions->instructions[i].encoded_size;

    return size;
}

static uint64_t align_up(uint64_t value, uint64_t alignment)
{
    assert(alignment > 0);
    return (value + alignment - 1) / alignment * alignment;
}

static void write_zero_padding(FILE* file, size_t padding_size)
{
    assert(file);

    uint8_t zeroes[ELF_PAGE_SIZE] = {};
    while (padding_size > 0)
    {
        size_t chunk_size = padding_size < sizeof(zeroes) ? padding_size : sizeof(zeroes);
        size_t bytes_written = fwrite(zeroes, 1, chunk_size, file);
        assert(bytes_written == chunk_size);
        padding_size -= chunk_size;
    }
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
    header->e_entry     = entry_point;          // first instruction address
    header->e_phoff     = sizeof(Elf64_Ehdr);   // Program Header offset (after ELF header)
    header->e_shoff     = 0;                    // Section Header offset
    header->e_flags     = 0;
    header->e_ehsize    = sizeof(Elf64_Ehdr);   // size of ELF header
    header->e_phentsize = sizeof(Elf64_Phdr);   // size of Program Header element
    header->e_phnum     = SEGMENT_COUNT;        // number of segments
    header->e_shentsize = 0;                    // no Section Header table
    header->e_shnum     = 0;                    // no sections
    header->e_shstrndx  = SHN_UNDEF;            // no section-name string table
}

void fill_program_headers(Elf64_Phdr* headers, uint64_t base_vaddr,
                          uint64_t text_file_size, uint64_t rodata_offset,
                          uint64_t rodata_size)
{
    assert(headers);

    memset(headers, 0, SEGMENT_COUNT * sizeof(Elf64_Phdr));

    // first segment (.text):
    headers[0].p_type   = PT_LOAD;          // load segment into memory
    headers[0].p_flags  = PF_R | PF_X;      // read + execute
    headers[0].p_offset = 0;                // segment starts at the beginning of the file
    headers[0].p_vaddr  = base_vaddr;       // virtual address of the segment in memory
    headers[0].p_paddr  = base_vaddr;       // physical address (ignored by Linux)
    headers[0].p_filesz = text_file_size;   // segment size in the file
    headers[0].p_memsz  = text_file_size;   // segment size in memory (no .bss)
    headers[0].p_align  = ELF_PAGE_SIZE;    // page alignment

    // second segment (.rodata):
    headers[1].p_type   = PT_LOAD;                    // load segment into memory
    headers[1].p_flags  = PF_R;                       // read only
    headers[1].p_offset = rodata_offset;              // segment offset in the file
    headers[1].p_vaddr  = base_vaddr + rodata_offset; // virtual address of the segment in memory
    headers[1].p_paddr  = base_vaddr + rodata_offset; // physical address (ignored by Linux)
    headers[1].p_filesz = rodata_size;                // segment size in the file
    headers[1].p_memsz  = rodata_size;                // segment size in memory (no .bss)
    headers[1].p_align  = ELF_PAGE_SIZE;              // page alignment
}
