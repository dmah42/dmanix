#ifndef MEMORY_H
#define MEMORY_H

#include "base.h"

namespace memory {

void* copy(void* dest, const void* src, uint32_t n);
void* set(void* dest, uint8_t c, uint32_t n);

uint8_t* copy(uint8_t* dest, const uint8_t* src, uint32_t n);
uint8_t* set(uint8_t* dest, uint8_t c, uint32_t n);

uint16_t* copy(uint16_t* dest, const uint16_t* src, uint32_t n);
uint16_t* set(uint16_t* dest, uint16_t c, uint32_t n);

}  // namespace memory

#endif  // MEMORY_H
