#ifndef ARRAY_SIZE_H
#define ARRAY_SIZE_H

#include <stdlib.h>

template <size_t N> struct ArraySizeHelper { char _[N]; };
template <typename T, size_t N> ArraySizeHelper<N> makeArraySizeHelper(T(&)[N]);
#define ARRAY_SIZE(a) sizeof(makeArraySizeHelper(a))

#endif  // ARRAY_SIZE_H
