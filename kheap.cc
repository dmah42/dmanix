#include "kheap.h"

#include "screen.h"

// end is defined in the linker script.
extern uint32_t end;

namespace kheap {

uint32_t base_address = (uint32_t)&end;

namespace {

uint32_t alloc_internal(uint32_t size, bool page_align, uint32_t* phys) {
  if (page_align && (base_address & 0xFFFFF000)) {
    base_address &= 0xFFFFF000;
    base_address += 0x1000;
  }
  if (phys != 0)
    *phys = base_address;
  uint32_t mem = base_address;
  base_address += size;
  return mem;
}

}  // namespace

uint32_t alloc(uint32_t size) {
  return alloc_internal(size, false, 0);
}

uint32_t alloc_p(uint32_t size, uint32_t* phys) {
  return alloc_internal(size, false, phys);
}

uint32_t alloc_pa(uint32_t size, uint32_t* phys) {
  return alloc_internal(size, true, phys);
}

}  // namespace kheap
