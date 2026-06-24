#include <assert.h>
#include <elf.h>
#include <string.h>

#include "asm_to_binary.h"

void asm_to_binary(FILE* const asm_file, FILE* const binary_file)
{
    assert(asm_file);
    assert(binary_file);

    Elf64_Ehdr elf_header = {};
    fill_elf_header(&elf_header, 0x400000);

    Elf64_Phdr program_header[SEGMENT_COUNT] = {};
    fill_program_headers(program_header);
}

void fill_elf_header(Elf64_Ehdr* header, uint64_t entry_point)
{
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

    header->e_type      = ET_EXEC;              // executable file
    header->e_machine   = EM_X86_64;            // architecture (x86-64)
    header->e_version   = EV_CURRENT;           // object file version
    header->e_entry     = entry_point;
    header->e_phoff     = sizeof(Elf64_Ehdr);   // Program Header offset (after ELF header)
    header->e_shoff     = 0;                    // !Section Header Table offset
    header->e_flags     = 0;
    header->e_ehsize    = sizeof(Elf64_Ehdr);   // size of ELF header
    header->e_phentsize = sizeof(Elf64_Phdr);   // size of Program Header Table element
    header->e_phnum     = SEGMENT_COUNT;        // number of segments
    header->e_shentsize = sizeof(Elf64_Shdr);   // size of Section Header Table element
    header->e_shnum     = 0;                    // !number of sections
    header->e_shstrndx  = 0;                    // !section with string table index
}

void fill_program_headers(Elf64_Phdr* headers, uint64_t base_vaddr, uint64_t code_offset, uint64_t code_size,
                                                                    uint64_t data_offset, uint64_t data_size)
{
    memset(headers, 0, 2 * sizeof(Elf64_Phdr));

    // first segment (.text):
    headers[0].p_type   = PT_LOAD;                  // load segment to memory
    headers[0].p_flags  = PF_R | PF_X;              // read + execute
    headers[0].p_offset = 0;                        // segment offset
    headers[0].p_vaddr  = base_vaddr;               // virtual address
    headers[0].p_paddr  = base_vaddr;               // physical address (basically ignored)
    headers[0].p_filesz = code_offset + code_size;  // size of segment in file
    headers[0].p_memsz  = code_offset + code_size;  // size of segment in memory (no .bss)
    headers[0].p_align  = 0x1000;                   // page size

    // second segment (.data):
    headers[1].p_type   = PT_LOAD;                  // load segment to memory
    headers[1].p_flags  = PF_R | PF_W;              // read + write
    headers[1].p_offset = data_offset;              // segment offset
    headers[1].p_vaddr  = base_vaddr + data_offset; // virtual address
    headers[1].p_paddr  = base_vaddr + data_offset; // physical address (basically ignored)
    headers[1].p_filesz = data_size;                // size of segment in file
    headers[1].p_memsz  = data_size;                // size of segment in memory (no .bss)
    headers[1].p_align  = 0x1000;                   // page size
}
