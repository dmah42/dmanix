#ifndef KALLOC_H
#define KALLOC_H

void* kalloc(uint32_t size);
void* kalloc_p(uint32_t size, uint32_t* phys = 0);
void* kalloc_pa(uint32_t size, uint32_t* phys = 0);
void kfree(void* p);

#endif  // KALLOC_H
