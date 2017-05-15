#ifndef KALLOC_H
#define KALLOC_H

#include <stdint.h>
#include <stdlib.h>

void* kalloc(uint32_t size);
void* kalloc_p(uint32_t size, uint32_t* phys = NULL);
void* kalloc_pa(uint32_t size, uint32_t* phys = NULL);
void kfree(void* p);

void* ualloc(uint32_t size);
void ufree(void* p);

namespace memory {
extern uint32_t base_address;
}  // memory

#endif  // KALLOC_H
