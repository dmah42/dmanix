#include "heap.h"
#include "paging.h"

// end is defined in the linker script.
extern uint32_t end;

namespace memory {
extern Heap* kheap;
extern Heap* uheap;

uint32_t base_address = (uint32_t)&end;

namespace {

void* alloc_internal(Heap* heap, uint32_t size, bool page_align,
                     uint32_t* phys) {
  if (heap != NULL) {
    uint32_t addr = (uint32_t) heap->Alloc(size, page_align);
    if (phys != NULL)
      *phys = GetPhysicalAddress(addr);
    return (void*) addr;
  } else {
    // TODO: align macro/method
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

void free_internal(Heap* heap, void* p) {
  if (heap != NULL) {
    if (heap->Owns(p))
      heap->Free(p);
  }
}

}  // namespace
}  // namespace memory

using namespace memory;

void* kalloc(uint32_t size) {
  return alloc_internal(kheap, size, false, NULL);
}

void* kalloc_p(uint32_t size, uint32_t* phys) {
  return alloc_internal(kheap, size, false, phys);
}

void* kalloc_pa(uint32_t size, uint32_t* phys) {
  return alloc_internal(kheap, size, true, phys);
}

void kfree(void* p) {
  free_internal(kheap, p);
}

void* ualloc(uint32_t size) {
  return alloc_internal(uheap, size, false, NULL);
}

void ufree(void* p) {
  free_internal(uheap, p);
}
