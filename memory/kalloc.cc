#include "heap.h"
#include "paging.h"

// end is defined in the linker script.
extern uint32_t end;

namespace memory {
extern Heap* kheap;

uint32_t base_address = (uint32_t)&end;

namespace {

void* kalloc_internal(uint32_t size, bool page_align, uint32_t* phys) {
  if (kheap != NULL) {
    uint32_t addr = (uint32_t) kheap->Alloc(size, page_align);
    if (phys != NULL)
      *phys = memory::GetPhysicalAddress(addr);
    return (void*) addr;
  } else {
    if (page_align && (base_address & 0xFFFFF000) != 0) {
      base_address &= 0xFFFFF000;
      base_address += 0x1000;
    }
    if (phys != NULL)
      *phys = base_address;
    uint32_t mem = base_address;
    base_address += size;
    return (void*) mem;
  }
}

}  // namespace
}  // namespace memory

using namespace memory;

void* kalloc(uint32_t size) {
  return kalloc_internal(size, false, NULL);
}

void* kalloc_p(uint32_t size, uint32_t* phys) {
  return kalloc_internal(size, false, phys);
}

void* kalloc_pa(uint32_t size, uint32_t* phys) {
  return kalloc_internal(size, true, phys);
}

void kfree(void* p) {
  if (kheap != NULL) {
    if (p == kheap)
      kheap = NULL;
    else if (kheap->Owns(p))
      kheap->Free(p);
  }
}
