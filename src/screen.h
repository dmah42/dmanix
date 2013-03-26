#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

enum Color {
  COLOR_BLACK = 0,
  COLOR_DARK_BLUE = 1,
  COLOR_DARK_GREEN = 2,
  COLOR_DARK_CYAN = 3,
  COLOR_DARK_RED = 4,
  COLOR_DARK_MAGENTA = 5,
  COLOR_DARK_YELLOW = 6,
  COLOR_LIGHT_GREY = 7,
  COLOR_GREY = 8,
  COLOR_BLUE = 9,
  COLOR_GREEN = 10,
  COLOR_CYAN = 11,
  COLOR_RED = 12,
  COLOR_MAGENTA = 13,
  COLOR_YELLOW = 14,
  COLOR_WHITE = 15
};

namespace screen {

void Initialize();

void Clear();

void puts(const char* s);
void Printf(const char* format, ...);

void ResetColor();
void SetColor(Color fore, Color back);

// Mode 13 stuff
void Mode13h();

void Pixel(uint16_t x, uint16_t y, Color color);
void Line(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, Color c);

}  // namespace screen

#endif  // SCREEN_H
