#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "asm_to_binary.h"
#include "asm_dump.h"

static char* read_file_to_buffer(FILE* const tree_txt_file);

void asm_to_binary(FILE* const asm_file, FILE* const binary_file)
{
    assert(asm_file);
    assert(binary_file);

    char* asm_buffer = read_file_to_buffer(asm_file);

    instruction_list_t instruction_list = {};
    label_list_t label_list = {};
    asm_code_to_instructions(asm_buffer, &instruction_list, &label_list);

    asm_dump(&instruction_list, &label_list, ASM_DUMP_TXT, ASM_DUMP_PNG);

    calculate_sizes(&instruction_list);
    compute_labels_addresses(&instruction_list, &label_list, BASE_VADDR + CODE_OFFSET);

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
