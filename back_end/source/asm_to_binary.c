#include <assert.h>
#include <elf.h>
#include <string.h>

#include "asm_to_binary.h"

void asm_to_binary(FILE* const asm_file, FILE* const binary_file)
{
    assert(asm_file);
    assert(binary_file);

    Elf64_Ehdr header = {};
    fill_elf_header(&header, 0x400000);
}

void fill_elf_header(Elf64_Ehdr* header, uint64_t entry_point)
{
    memset(header, 0, sizeof(Elf64_Ehdr));

    header->e_ident[EI_MAG0]        = ELFMAG0;          // 0x7F
    header->e_ident[EI_MAG1]        = ELFMAG1;          // 'E'
    header->e_ident[EI_MAG2]        = ELFMAG2;          // 'L'
    header->e_ident[EI_MAG3]        = ELFMAG3;          // 'F'
    header->e_ident[EI_CLASS]       = ELFCLASS64;
    header->e_ident[EI_DATA]        = ELFDATA2LSB;      // little-endian
    header->e_ident[EI_VERSION]     = EV_CURRENT;
    header->e_ident[EI_OSABI]       = ELFOSABI_LINUX;   // ABI
    header->e_ident[EI_ABIVERSION]  = 0;
    // remaining bytes of e_ident are zeroed out by memset

    header->e_type      = ET_EXEC;              // executable file
    header->e_machine   = EM_X86_64;            // architecture (x86-64)
    header->e_version   = EV_CURRENT;
    header->e_entry     = entry_point;
    header->e_phoff     = sizeof(Elf64_Ehdr);   // Program Header offset (after ELF header)
    header->e_shoff     = 0;                    // !Section Header Table offset
    header->e_flags     = 0;
    header->e_ehsize    = sizeof(Elf64_Ehdr);   // size of ELF header
    header->e_phentsize = sizeof(Elf64_Phdr);   // size of Program Header Table element
    header->e_phnum     = 2;                    // number of segments
    header->e_shentsize = sizeof(Elf64_Shdr);   // size of Section Header Table element
    header->e_shnum     = 0;                    // !number of sections
    header->e_shstrndx  = 0;                    // !section with string table index
}
