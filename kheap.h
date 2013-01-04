#ifndef KMALLOC_H
#define KMALLOC_H

#include "base.h"

namespace kheap {

uint32_t alloc(uint32_t size);
uint32_t alloc_p(uint32_t size, uint32_t* phys = 0);
uint32_t alloc_pa(uint32_t size, uint32_t* phys = 0);

}  // namespace kheap

#endif  // KMALLOC_H
