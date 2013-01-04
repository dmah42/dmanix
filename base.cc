#include "base.h"

#include "screen.h"

void panic(const char* message, const char* file, uint32_t line) {
  asm volatile("cli");

  screen::puts("PANIC(");
  screen::puts(message);
  screen::puts(") at ");
  screen::puts(file);
  screen::puts(":");
  screen::putd(line);
  screen::putc('\n');

  for(;;);
}
