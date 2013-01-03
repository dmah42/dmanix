#ifndef MEMORY_H
#define MEMORY_H

#include "base.h"

namespace memory {

template<typename T>
  T* copy(T* dest, const T* src, uint32_t n) {
    for (uint32_t i = 0; i < n; ++i)
      dest[i] = src[i];
    return dest;
  }

template<typename T>
  T* set(T* dest, T c, uint32_t n) {
    for (uint32_t i = 0; i < n; ++i)
      dest[i] = c;
    return dest;
  }

}  // namespace memory

#endif  // MEMORY_H
