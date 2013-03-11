#ifndef PROCESS_ELF32_PARSER_H_
#define PROCESS_ELF32_PARSER_H_

#include <stdint.h>
#include <stdlib.h>

namespace multiboot {
// TODO(dominic): merge section header structs.
struct ELFSectionHeaderTable;
}  // namespace multiboot

namespace process {

class Elf32Parser {
 public:
  struct SectionHeader;
   
  explicit Elf32Parser(const char* filename);
  explicit Elf32Parser(
      const multiboot::ELFSectionHeaderTable& elf_sec);
  ~Elf32Parser();

  uint32_t entry_point() const { return header_.entry; }

  void WriteAllSections();

  const char* FindSymbolName(uint32_t address, uint32_t* start_address) const;
  uint32_t FindSymbolAddress(const char* name) const;
  uint32_t FindDynamicSymbolAddress(uint32_t offset) const;
  const char* FindDynamicSymbolName(uint32_t offset) const;

  uint32_t GetLastAddress() const;
  uint32_t GetGlobalOffsetTable() const;

 private:
  struct Header {
    uint8_t ident[16];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstring_index;
  };
  struct Relocation;
  struct Symbol;
  struct Dynamic;

  const char* filename_;

  Header header_;
  SectionHeader* section_headers_;
  SectionHeader* symbol_table_;
  SectionHeader* string_table_;
  SectionHeader* global_offset_table_;
  SectionHeader* relocation_table_;
};

}  // namespace process

#endif  // PROCESS_ELF32_PARSER_H_
