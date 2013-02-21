#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

namespace memory {

uint8_t* copy8(uint8_t* dest, const uint8_t* src, uint32_t n);
uint8_t* set8(uint8_t* dest, uint8_t c, uint32_t n);

uint16_t* copy16(uint16_t* dest, const uint16_t* src, uint32_t n);
uint16_t* set16(uint16_t* dest, uint16_t c, uint32_t n);

uint32_t* copy32(uint32_t* dest, const uint32_t* src, uint32_t n);
uint32_t* set32(uint32_t* dest, uint32_t c, uint32_t n);

}  // namespace memory

#endif  // MEMORY_H
