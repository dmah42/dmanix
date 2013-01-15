#include "assert.h"

#include "screen.h"

void panic(const char* message, const char* file, uint32_t line) {
  asm volatile("cli");

  screen::Printf("PANIC(%s) at %s: %u\n", message, file, line);
  for(;;);
}
