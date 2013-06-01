#ifndef ASSERT_H
#define ASSERT_H

#include <stdint.h>

void panic(const char*, const  char*, uint32_t);
#define PANIC(msg)          panic(msg, __FILE__, __LINE__)
#define ASSERT_MSG(b, msg)  if (b) { void(0); } else { PANIC(msg); }
#define ASSERT(b)           ASSERT_MSG(b, #b)

#ifdef DEBUG
#define DEBUG_ASSERT(b)          ASSERT(b)
#define DEBUG_ASSERT_MSG(b, msg) ASSERT_MSG(b, msg)
#else
#define DEBUG_ASSERT(b)          do { (void)(b); } while (0)
#define DEBUG_ASSERT_MSG(b, msg) do { (void)(b); (void)(msg); } while (0)
#endif

#endif  // ASSERT_H
