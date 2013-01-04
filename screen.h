#ifndef SCREEN_H
#define SCREEN_H

#include "base.h"

enum Color {
  COLOR_BLACK = 0,
  COLOR_DARK_RED = 1,
  COLOR_DARK_GREEN = 2,
  COLOR_DARK_YELLOW = 3,
  COLOR_DARK_BLUE = 4,
  COLOR_DARK_MAGENTA = 5,
  COLOR_DARK_CYAN = 6,
  COLOR_LIGHT_GREY = 7,
  COLOR_GREY = 8,
  COLOR_RED = 9,
  COLOR_GREEN = 10,
  COLOR_YELLOW = 11,
  COLOR_BLUE = 12,
  COLOR_MAGENTA = 13,
  COLOR_CYAN = 14,
  COLOR_WHITE = 15
};

namespace screen {

void putc(char c);
void puts(const char* s);
void puth(uint32_t h);
void putd(uint32_t d);

void Clear();

void ResetColor();
void SetColor(Color fore, Color back);

}  // namespace screen

#endif  // SCREEN_H
