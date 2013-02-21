#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

namespace multiboot {

void Dump();

enum Flag {
  FLAG_MEM = 1 << 0, //0x001,
  FLAG_DEVICE = 1 << 1, // 0x002,
  FLAG_CMDLINE = 1 << 2, // 0x004,
  FLAG_MODS = 1 << 3, // 0x008,
  FLAG_AOUT = 1 << 4, // 0x010,
  FLAG_ELF = 1 << 5, // 0x020,
  FLAG_MMAP = 1 << 6, // 0x040,
  FLAG_CONFIG = 1 << 7, // 0x080,
  FLAG_LOADER = 1 << 8, // 0x100,
  FLAG_APM = 1 << 9, // 0x200,
  FLAG_VBE = 1 << 10, // 0x400
};

struct AOUTSymbolTable
{
  uint32_t tabsize;
  uint32_t strsize;
  uint32_t addr;
  uint32_t reserved;
} __attribute__((packed));

struct ELFSectionHeaderTable
{
  uint32_t num;
  uint32_t size;
  uint32_t addr;
  uint32_t shndx;
} __attribute__((packed));

struct Info {
  bool CheckFlag(Flag flag) const { return (flags & flag) != 0; }

  uint32_t flags;
  uint32_t mem_lower;
  uint32_t mem_upper;
  uint32_t boot_device;
  uint32_t cmdline;
  uint32_t mods_count;
  uint32_t mods_addr;
  union {
    AOUTSymbolTable aout_sym;
    ELFSectionHeaderTable elf_sec;
  } u;
  uint32_t mmap_length;
  uint32_t mmap_addr;
  uint32_t drives_length;
  uint32_t drives_addr;
  uint32_t config_table;
  uint32_t boot_loader_name;
  uint32_t apm_table;
  uint32_t vbe_control_info;
  uint32_t vbe_mode_info;
  uint32_t vbe_mode;
  uint32_t vbe_interface_seg;
  uint32_t vbe_interface_off;
  uint32_t vbe_interface_len;
}  __attribute__((packed));

struct MMapEntry {
  uint32_t size;
  uint64_t addr;
  uint64_t len;
  uint32_t type;
} __attribute__((packed));

struct Module {
  uint32_t start_address;
  uint32_t end_address;
  const char* cmdline;
  uint32_t pad;
} __attribute__((packed));

}  // namespace multiboot

#endif  // MULTIBOOT_H
