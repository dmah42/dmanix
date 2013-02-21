#ifndef ASSERT_H
#define ASSERT_H

#include <stdint.h>

void panic(const char*, const  char*, uint32_t);
#define PANIC(msg)  panic(msg, __FILE__, __LINE__)
#define ASSERT(b)   if (b) { void(0); } else { PANIC(#b); }

#endif  // ASSERT_H
