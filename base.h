#ifndef BASE_H
#define BASE_H

typedef unsigned int    uint32_t;
typedef          int    int32_t;
typedef unsigned short  uint16_t;
typedef          short  int16_t;
typedef unsigned char   uint8_t;
typedef          char   int8_t;

typedef uint32_t        size_t;

void panic(const char*, const  char*, uint32_t);
#define PANIC(msg)  panic(msg, __FILE__, __LINE__)

#endif  // BASE_H
