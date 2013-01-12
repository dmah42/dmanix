#ifndef BASE_H
#define BASE_H

#include <stdint.h>
#include <stdlib.h>

void panic(const char*, const  char*, uint32_t);
#define PANIC(msg)  panic(msg, __FILE__, __LINE__)
#define ASSERT(b)   if (b) { void(0); } else { PANIC(#b); }

template <size_t N> struct ArraySizeHelper { char _[N]; };
template <typename T, size_t N>
  ArraySizeHelper<N> makeArraySizeHelper(T(&)[N]);
#define ARRAY_SIZE(a) sizeof(makeArraySizeHelper(a))

#endif  // BASE_H
