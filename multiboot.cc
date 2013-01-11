#include "multiboot.h"

#include "base.h"
#include "screen.h"

// from boot.s
extern "C" uint32_t mbmagic;
extern "C" multiboot::Info* mbd;

namespace multiboot {

void Dump() {
  ASSERT(mbmagic == MULTIBOOT_BOOTLOADER_MAGIC);

  screen::Printf("flags = %x\n", mbd->flags);
  if (mbd->CheckFlag(multiboot::FLAG_MEM)) {
    screen::Printf("mem_lower = %uKB, mem_upper = %uKB\n",
                   mbd->mem_lower, mbd->mem_upper);
  }

  if (mbd->CheckFlag(multiboot::FLAG_DEVICE)) {
    screen::Printf("boot_device = %x\n", mbd->boot_device);
  }

  if (mbd->CheckFlag(multiboot::FLAG_CMDLINE)) {
    screen::Printf("cmdline = %s\n", (const char *) mbd->cmdline);
  }

  if (mbd->CheckFlag(multiboot::FLAG_MODS)) {
    screen::Printf("mods_count = %d, mods_addr = %x\n",
                   mbd->mods_count, mbd->mods_addr);
    const multiboot::Module* mod = (multiboot::Module*) mbd->mods_addr;
    for (uint32_t i = 0; i < mbd->mods_count; ++i, ++mod)
      screen::Printf("  mod_start = %x, mod_end = %x, cmdline = %s\n",
                     mod->start_address, mod->end_address, mod->cmdline);
  }

  // Bits 4 and 5 are mutually exclusive

  // Is the symbol table of a.out valid?
  if (mbd->CheckFlag(multiboot::FLAG_AOUT)) {
    ASSERT(!mbd->CheckFlag(multiboot::FLAG_ELF));
    multiboot::AOUTSymbolTable* aout_sym = &(mbd->u.aout_sym);

    screen::Printf("aout_symbol_table: tabsize = %x, strsize = %x, addr = %x\n",
                   aout_sym->tabsize, aout_sym->strsize, aout_sym->addr);
  }

  // Is the section header table of ELF valid?
  if (mbd->CheckFlag(multiboot::FLAG_ELF)) {
    multiboot::ELFSectionHeaderTable* elf_sec = &(mbd->u.elf_sec);

    screen::Printf("elf_sec: num = %u, size = %x, addr = %x, shndx = %x\n",
                   elf_sec->num, elf_sec->size, elf_sec->addr, elf_sec->shndx);
  }

  // Are mmap_* valid?
  if (mbd->CheckFlag(multiboot::FLAG_MMAP)) {
    screen::Printf("mmap_addr = %x, mmap_length = %x\n",
                   mbd->mmap_addr, mbd->mmap_length);
    for (multiboot::MMapEntry* mmap = (multiboot::MMapEntry*) mbd->mmap_addr;
        (uint64_t) mmap < mbd->mmap_addr + mbd->mmap_length;
        mmap = (multiboot::MMapEntry*) ((uint64_t) mmap + mmap->size + sizeof(mmap->size))) {
      screen::Printf(" size = %x, base_addr = %x.%x, length = %x.%x, type = %x\n",
                     mmap->size, mmap->addr >> 32, mmap->addr & 0xFFFFFFFF,
                     mmap->len >> 32, mmap->len & 0xFFFFFFFF, mmap->type);
    }
  }
}

}  // namespace multiboot
