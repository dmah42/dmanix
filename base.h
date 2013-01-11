#ifndef BASE_H
#define BASE_H

#include <stdint.h>

void panic(const char*, const  char*, uint32_t);
#define PANIC(msg)  panic(msg, __FILE__, __LINE__)

#endif  // BASE_H
