#ifndef ASSERT_H
#define ASSERT_H

#include <stdint.h>

void panic(const char*, const  char*, uint32_t);
#define PANIC(msg)          panic(msg, __FILE__, __LINE__)
#define ASSERT_MSG(b, msg)  if (b) { void(0); } else { PANIC(msg); }
#define ASSERT(b)           ASSERT_MSG(b, #b)

#ifdef DEBUG
#define DEBUG_ASSERT(b) ASSERT(b)
#else
#define DEBUG_ASSERT(b) (void)(b);
#endif

#endif  // ASSERT_H
