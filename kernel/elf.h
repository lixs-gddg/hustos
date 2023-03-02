#ifndef _ELF_H_
#define _ELF_H_

#include "util/types.h"
#include "process.h"

#define MAX_CMDLINE_ARGS 64

//add for lab1_challenge1_backtrace
/* Legal values for sh_type (section type).  */
#define SHT_NULL	  0		/* Section header table entry unused */
#define SHT_PROGBITS	  1		/* Program data */
#define SHT_SYMTAB	  2		/* Symbol table */
#define SHT_STRTAB	  3		/* String table */
#define SHT_RELA	  4		/* Relocation entries with addends */
#define SHT_HASH	  5		/* Symbol hash table */
#define SHT_DYNAMIC	  6		/* Dynamic linking information */
#define SHT_NOTE	  7		/* Notes */
#define SHT_NOBITS	  8		/* Program space with no data (bss) */
#define SHT_REL		  9		/* Relocation entries, no addends */
#define SHT_SHLIB	  10		/* Reserved */
#define SHT_DYNSYM	  11		/* Dynamic linker symbol table */

/* Legal values for st_info (symbol type).  */
#define STT_NOTYPE  0
#define STT_OBJECT  1
#define STT_FUNC    2
#define STT_SECTION 3
#define STT_FILE    4
#define STT_COMMON  5
#define STT_TLS     6

// elf header structure
typedef struct elf_header_t {
  uint32 magic;
  uint8 elf[12];
  uint16 type;      /* Object file type */
  uint16 machine;   /* Architecture */
  uint32 version;   /* Object file version */
  uint64 entry;     /* Entry point virtual address */
  uint64 phoff;     /* Program header table file offset */
  uint64 shoff;     /* Section header table file offset */
  uint32 flags;     /* Processor-specific flags */
  uint16 ehsize;    /* ELF header size in bytes */
  uint16 phentsize; /* Program header table entry size */
  uint16 phnum;     /* Program header table entry count */
  uint16 shentsize; /* Section header table entry size */
  uint16 shnum;     /* Section header table entry count */
  uint16 shstrndx;  /* Section header string table index */
} elf_header;

// Program segment header.
typedef struct elf_prog_header_t {
  uint32 type;   /* Segment type */
  uint32 flags;  /* Segment flags */
  uint64 off;    /* Segment file offset */
  uint64 vaddr;  /* Segment virtual address */
  uint64 paddr;  /* Segment physical address */
  uint64 filesz; /* Segment size in file */
  uint64 memsz;  /* Segment size in memory */
  uint64 align;  /* Segment alignment */
} elf_prog_header;

// added for lab1_challenge1_backtrace.
// elf section header
typedef struct elf_sect_header_t{
  uint32 sh_name;      /* Section name (string tbl index) */
  uint32 sh_type;      /* Section type */
  uint64 sh_flags;     /* Section flags */
  uint64 sh_addr;      /* Section virtual addr at execution */
  uint64 sh_offset;    /* Section file offset */
  uint64 sh_size;      /* Section size in bytes */
  uint32 sh_link;      /* Link to another section */
  uint32 sh_info;      /* Additional section information */
  uint64 sh_addralign; /* Section alignment */
  uint64 sh_entsize;   /* Entry size if section holds table */
} elf_sect_header;

// added for lab1_challenge1_backtrace.
// elf section symbol struct;
typedef struct elf_symbol_t{
    uint32        st_name;    /* Symbol name, index in string tbl */
    unsigned char st_info;    /* Type and binding attributes */
    unsigned char st_other;   /* No defined meaning, 0 */
    uint16        st_shndx;   /* Associated section index */
    uint64        st_value;   /* Value of the symbol */
    uint64        st_size;    /* Associated symbol size */
}elf_symbol;

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian
#define ELF_PROG_LOAD 1

typedef enum elf_status_t {
  EL_OK = 0,

  EL_EIO,
  EL_ENOMEM,
  EL_NOTELF,
  EL_ERR,

} elf_status;

typedef struct elf_ctx_t {
  void *info;
  elf_header ehdr;
  // added for lab1_challenge1_backtrace.
  //Assume that the length of strtab is 4096 at most.
  char strtab[4096];
  //Assume that there are only 128 symbols at most.
  elf_symbol symtab[128];
  //actual length of the symtab.
  uint64 symtab_length;
} elf_ctx;

elf_status elf_init(elf_ctx *ctx, void *info);
elf_status elf_load(elf_ctx *ctx);
// added for lab1_challenge1_backtrace.
elf_status elf_load_symbol_string(elf_ctx *ctx);

void load_bincode_from_host_elf(process *p);

#endif
