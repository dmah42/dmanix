#include "process/elf32_parser.h"

#include "base/assert.h"
#include "fs/fs.h"
#include "fs/node.h"
#include "memory/kalloc.h"
#include "memory/memory.h"
#include "memory/paging.h"
#include "multiboot.h"
#include "string.h"

#define SHT_PROGBITS      0x1     // The data is contained in the program file.
#define SHT_SYMTAB	      0x2	  // Symbol table
#define SHT_STRTAB	      0x3	  // String table
#define SHT_RELA          0x4
#define SHT_HASH          0x5     // Symbol hash table
#define SHT_DYNAMIC       0x6     // Dynamic linking information
#define SHT_NOTE          0x7
#define SHT_NOBITS        0x8     // The data is not contained in the program file.
#define SHT_REL           0x9
#define SHT_DYNSYM        0xb
#define SHT_INIT_ARRAY    0xe
#define SHT_FINI_ARRAY    0xf
#define SHT_PREINIT_ARRAY 0x10

#define SHF_WRITE         0x1
#define SHF_ALLOC         0x2
#define SHF_EXECINSTR     0x4
#define SHF_MASKPROC      0xf0000000

#define ELF32_R_SYM(val)  ((val) >> 8)
#define ELF32_R_TYPE(val) ((val) & 0xff)

namespace process {

struct Elf32Parser::SectionHeader {
  uint32_t name;
  uint32_t type;
  uint32_t flags;
  uint32_t address;
  uint32_t offset;
  uint32_t size;
  uint32_t link;
  uint32_t info;
  uint32_t address_align;
  uint32_t entry_size;
};

struct Elf32Parser::Relocation {
  uint32_t offset;
  uint32_t info;
};

struct Elf32Parser::Symbol {
  uint32_t name;
  uint32_t value;
  uint32_t size;
  uint8_t info;
  uint8_t other;
  uint16_t shndx;
};

struct Elf32Parser::Dynamic {
  int32_t tag;
  union
  {
    int32_t val;
    uint32_t ptr;
  } un;
};

Elf32Parser::Elf32Parser(const char* filename)
    : filename_(filename),
      section_headers_(NULL),
      symbol_table_(NULL),
      string_table_(NULL),
      global_offset_table_(NULL),
      relocation_table_(NULL) {
  fs::Node* file = fs::Traverse(filename);
  ASSERT(file != NULL);

  file->Open();

  uint32_t result = file->Read(0, sizeof(Header),
                               reinterpret_cast<uint8_t*>(&header_));
  ASSERT(result == sizeof(Header));

  section_headers_ = reinterpret_cast<SectionHeader*>(
      kalloc(sizeof(SectionHeader) * header_.shnum));

  ASSERT(header_.shentsize == sizeof(SectionHeader));
  result = file->Read(header_.shoff, header_.shnum * header_.shentsize,
                      reinterpret_cast<uint8_t*>(&section_headers_));
  ASSERT(result == static_cast<uint32_t>(header_.shnum * header_.shentsize));

  string_table_ = &(section_headers_[header_.shstring_index]);
  
  for (uint32_t i = 0; i < header_.shnum; ++i) {
    if (section_headers_[i].type == SHT_SYMTAB) {
      symbol_table_ = &(section_headers_[i]);
      break;
    }
  }

  // Load the string table
  uint8_t* strtab = reinterpret_cast<uint8_t*>(ualloc(string_table_->size));
  file->Read(string_table_->offset, string_table_->size, strtab);
  for (uint32_t i = 0; i < header_.shnum; ++i) {
    const char* str = reinterpret_cast<const char*>(
        strtab + section_headers_[i].name);
    if (string::compare(str, ".got.plt"))
      global_offset_table_ = &(section_headers_[i]);
    else if (string::compare(str, ".rel.plt"))
      relocation_table_ = &(section_headers_[i]);
  }
  ufree(strtab);
  file->Close();
}

Elf32Parser::Elf32Parser(
    const multiboot::ELFSectionHeaderTable& elf_sec)
    : filename_(NULL),
      section_headers_(NULL),
      symbol_table_(NULL),
      string_table_(NULL),
      global_offset_table_(NULL),
      relocation_table_(NULL) {
  SectionHeader* shstrtab = reinterpret_cast<SectionHeader*>(
      elf_sec.addr + elf_sec.shndx * elf_sec.size);
  for (uint32_t i = 0; i < elf_sec.num; ++i) {
    SectionHeader* sh = reinterpret_cast<SectionHeader*>(
        elf_sec.addr + i * elf_sec.size);
    if (sh->type == SHT_SYMTAB) {
      symbol_table_ = sh;
    } else if (sh->type == SHT_STRTAB) {
      const char* c = reinterpret_cast<const char*>(
          shstrtab->address + sh->name);
      if (string::compare(c, ".strtab"))
        string_table_ = sh;
    }
  }
  ASSERT(symbol_table_ != NULL && string_table_ != NULL);
}

Elf32Parser::~Elf32Parser() {
  if (filename_)
    kfree(section_headers_);
}

void Elf32Parser::WriteAllSections() {
  fs::Node* file = fs::Traverse(filename_);
  DEBUG_ASSERT(file);
  file->Open();

  for (uint32_t i = 0; i < header_.shnum; ++i) {
    if (section_headers_[i].flags & SHF_ALLOC) {
      memory::AllocateRange(section_headers_[i].address,
                            section_headers_[i].size);
      if (section_headers_[i].type == SHT_NOBITS) {
        memory::set8(reinterpret_cast<uint8_t*>(section_headers_[i].address),
                     0,
                     section_headers_[i].size);
      } else {
        uint32_t result = file->Read(
            section_headers_[i].offset, section_headers_[i].size,
            reinterpret_cast<uint8_t*>(section_headers_[i].address));
        ASSERT(result == section_headers_[i].size);
      }
    }
  }
}

const char* Elf32Parser::FindSymbolName(uint32_t address,
                                        uint32_t* start_address) const {
  Symbol* symbol = reinterpret_cast<Symbol*>(symbol_table_->address);

  for (uint32_t i = 0; i < symbol_table_->size / sizeof(Symbol); ++i) {
    if (address >= symbol->value &&
        address < symbol->value + symbol->size) {
      if (start_address)
        *start_address = symbol->value;
      const char* symbol_name =
          reinterpret_cast<const char*>(symbol->name) + string_table_->address;
      return symbol_name;
    }
    ++symbol;
  }
  return NULL;
}

uint32_t Elf32Parser::FindSymbolAddress(const char* name) const {
  Symbol* symbol = reinterpret_cast<Symbol*>(symbol_table_->address);
  
  for (uint32_t i = 0; i < symbol_table_->size / sizeof(Symbol); ++i) {
    const char* symbol_name =
        reinterpret_cast<const char*>(symbol->name) + string_table_->address;
    if (string::compare(name, symbol_name))
      return symbol->value;
    ++symbol;
  }
  return 0;
}

uint32_t Elf32Parser::FindDynamicSymbolAddress(uint32_t offset) const {
  SectionHeader* sh = relocation_table_;
  if (sh->type == SHT_REL)
    return reinterpret_cast<Relocation*>(sh->address + offset)->offset;
  return 0;
}

const char* Elf32Parser::FindDynamicSymbolName(uint32_t offset) const {
  SectionHeader* sh = relocation_table_;
  if (sh->type == SHT_REL) {
    Relocation* rel = reinterpret_cast<Relocation*>(sh->address + offset);
    const SectionHeader& dyn = section_headers_[sh->link];
    Dynamic* tag = reinterpret_cast<Dynamic*>(
        dyn.address + ELF32_R_SYM(rel->info) * dyn.entry_size);
    const SectionHeader& strtab = section_headers_[dyn.link];
    return reinterpret_cast<const char*>(strtab.address + tag->tag);
  }
  return NULL;
}

uint32_t Elf32Parser::GetLastAddress() const {
  uint32_t max = 0;
  for (uint32_t i = 0; i < header_.shnum; ++i) {
    if (section_headers_[i].flags & SHF_ALLOC) {
      uint32_t address = section_headers_[i].address + section_headers_[i].size;
      if (address > max)
        max = address;
    }
  }
  return max;
}

uint32_t Elf32Parser::GetGlobalOffsetTable() const {
  return global_offset_table_ != NULL ? global_offset_table_->address : 0;
}

}  // namespace process
