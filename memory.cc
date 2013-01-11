#include "memory.h"

namespace memory {
namespace {

template<typename T>
  T* copy_internal(T* dest, const T* src, uint32_t n) {
    ASSERT(dest != 0);
    ASSERT(src != 0);
    for (uint32_t i = 0; i < n; ++i)
      dest[i] = src[i];
    return dest;
  }

template<typename T>
  T* set_internal(T* dest, T c, uint32_t n) {
    ASSERT(dest != 0);
    for (uint32_t i = 0; i < n; ++i)
      dest[i] = c;
    return dest;
  }
}  // end namespace

void* copy(void* dest, const void* src, uint32_t n) {
  return copy((uint8_t*) dest, (const uint8_t*) src, n);
}

void* set(void* dest, uint8_t c, uint32_t n) {
  return set((uint8_t*) dest, c, n);
}

uint8_t* copy(uint8_t* dest, const uint8_t* src, uint32_t n) {
  return copy_internal(dest, src, n);
}

uint8_t* set(uint8_t* dest, uint8_t c, uint32_t n) {
  return set_internal(dest, c, n);
}

uint16_t* copy(uint16_t* dest, const uint16_t* src, uint32_t n) {
  return copy_internal(dest, src, n);
}

uint16_t* set(uint16_t* dest, uint16_t c, uint32_t n) {
  return set_internal(dest, c, n);
}

}  // endif memory
