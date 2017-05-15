#include "memory/heap.h"
#include "memory/paging.h"

// end is defined in the linker script.
extern uint32_t end;

namespace memory {

uint32_t base_address = (uint32_t)&end;

namespace {

void* alloc_internal(Heap* heap, uint32_t size, bool page_align,
                     uint32_t* phys) {
  uint32_t addr = 0;
  if (heap != NULL) {
    addr = reinterpret_cast<uint32_t>(heap->Alloc(size, page_align));
    if (phys != NULL)
      *phys = GetPhysicalAddress(addr);
  } else {
    // TODO(dominic): align macro/method
    if (page_align && (base_address & 0xFFFFF000) != 0) {
      base_address &= 0xFFFFF000;
      base_address += 0x1000;
    }
    if (phys != NULL)
      *phys = base_address;
    addr = base_address;
    base_address += size;
  }
  return reinterpret_cast<void*>(addr);
}

void free_internal(Heap* heap, void* p) {
  if (heap != NULL) {
    if (heap->Owns(p))
      heap->Free(p);
  }
}

}  // namespace
}  // namespace memory

using memory::alloc_internal;
using memory::free_internal;
using memory::kheap;
using memory::uheap;

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
