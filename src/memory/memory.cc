#include "memory.h"

#include <stdlib.h>

#include "base/assert.h"

namespace memory {
namespace {

template<typename T>
  T* copy_internal(T* dest, const T* src, uint32_t n) {
    ASSERT(dest != NULL);
    ASSERT(src != NULL);
    for (uint32_t i = 0; i < n; ++i)
      dest[i] = src[i];
    return dest;
  }

template<typename T>
  T* set_internal(T* dest, T c, uint32_t n) {
    ASSERT(dest != NULL);
    for (uint32_t i = 0; i < n; ++i)
      dest[i] = c;
    return dest;
  }
}  // end namespace

uint8_t* copy8(uint8_t* dest, const uint8_t* src, uint32_t n) {
  return copy_internal(dest, src, n);
}

uint8_t* set8(uint8_t* dest, uint8_t c, uint32_t n) {
  return set_internal(dest, c, n);
}

uint16_t* copy16(uint16_t* dest, const uint16_t* src, uint32_t n) {
  return copy_internal(dest, src, n);
}

uint16_t* set16(uint16_t* dest, uint16_t c, uint32_t n) {
  return set_internal(dest, c, n);
}

uint32_t* copy32(uint32_t* dest, const uint32_t* src, uint32_t n) {
  return copy_internal(dest, src, n);
}

uint32_t* set32(uint32_t* dest, uint32_t c, uint32_t n) {
  return set_internal(dest, c, n);
}

}  // namespace memory
